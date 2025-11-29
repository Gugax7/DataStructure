#include "btree.h"
#include <stdlib.h>
#include <string.h>

// Add these constants at the top of your file if not already defined

#define TEMP_DATABASE "temp_data.bin"
#define TEMP_INDEX_BTREE "temp_index.btree"

// --- Helper: Read/Write Nodes ---

int compare_keys(const char* name1, int thresh1, const char* name2, int thresh2) {
    int nameCmp = strcmp(name1, name2);
    if (nameCmp != 0) return nameCmp;
    
    if (thresh1 < thresh2) return -1;
    if (thresh1 > thresh2) return 1;
    
    return 0; // Exact match
}

int compare_records(const char* keyName, int keyThreshold, IndexRecord existingRecord) {
    // 1. Compare Names first
    int nameCmp = strncmp(keyName, existingRecord.recordName, 50);
    
    // 2. If names are different, return that result
    if (nameCmp != 0) {
        return nameCmp;
    }
    
    // 3. If names are EQUAL, compare thresholds
    // This allows "img.pgm" (100) and "img.pgm" (150) to exist together
    if (keyThreshold < existingRecord.thresholdValue) return -1;
    if (keyThreshold > existingRecord.thresholdValue) return 1;
    
    return 0; // Exactly the same record
}

void read_node(FILE *f, long offset, BTreeNode *node) {
    if (offset == INVALID_OFFSET) return;
    fseek(f, offset, SEEK_SET);
    fread(node, sizeof(BTreeNode), 1, f);
}

void write_node(FILE *f, long offset, BTreeNode *node) {
    fseek(f, offset, SEEK_SET);
    fwrite(node, sizeof(BTreeNode), 1, f);
}

void read_header(FILE *f, BTreeHeader *header) {
    fseek(f, 0, SEEK_SET);
    fread(header, sizeof(BTreeHeader), 1, f);
}

void write_header(FILE *f, BTreeHeader *header) {
    fseek(f, 0, SEEK_SET);
    fwrite(header, sizeof(BTreeHeader), 1, f);
}

long get_new_node_offset(FILE *f, BTreeHeader *header) {
    long offset = header->nextFreeOffset;
    header->nextFreeOffset += sizeof(BTreeNode);
    write_header(f, header); 
    return offset;
}

// --- Management Functions ---

FILE* btree_open(const char* filename) {
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        // Create new if doesn't exist
        f = fopen(filename, "wb+");
        if (!f) return NULL;

        BTreeHeader header;
        header.rootNodeOffset = sizeof(BTreeHeader);
        header.nextFreeOffset = header.rootNodeOffset + sizeof(BTreeNode);
        write_header(f, &header);

       BTreeNode root;
    memset(&root, 0, sizeof(BTreeNode)); // Zera tudo
    
    // CORREÇÃO CRÍTICA: Definir filhos como inválidos (-1)
    for(int i=0; i < 2*BTREE_ORDER; i++) {
        root.children[i] = INVALID_OFFSET; 
    }

    root.isLeaf = 1;
    root.numKeys = 0;
    write_node(f, header.rootNodeOffset, &root);
    }
    return f;
}

void btree_close(FILE* treeFile) {
    if (treeFile) fclose(treeFile);
}

// --- Search Logic ---

int btree_search(FILE* treeFile, const char* key, int threshold, IndexRecord* foundRecord) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    long currentOffset = header.rootNodeOffset;
    BTreeNode node;

    while (currentOffset != INVALID_OFFSET) {
        read_node(treeFile, currentOffset, &node);
        
        int i = 0;
        // Use the new helper here too!
        while (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
            i++;
        }

        // Check if equal (Name == Name AND Threshold == Threshold)
        if (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) == 0) {
            if (foundRecord) *foundRecord = node.records[i];
            return 1; // Found exact match
        }

        if (node.isLeaf) return 0; // Not found
        currentOffset = node.children[i];
    }
    return 0;
}

// --- Insert Logic (Splitting) ---

// 
void btree_split_child(FILE *f, long parentOffset, BTreeNode *parent, int i, BTreeHeader *header) {
    long childOffset = parent->children[i];
    BTreeNode child;
    read_node(f, childOffset, &child);

    // Create 'z' (new node)
    long newChildOffset = get_new_node_offset(f, header);
    BTreeNode newChild;
    memset(&newChild, 0, sizeof(BTreeNode));
    
    // --- FIX START: INITIALIZE POINTERS TO -1 ---
    for(int k=0; k < 2*BTREE_ORDER; k++) {
        newChild.children[k] = INVALID_OFFSET;
    }
    // --- FIX END ---

    newChild.isLeaf = child.isLeaf;
    newChild.numKeys = BTREE_ORDER - 1;

    // Copy top half of keys to newChild
    for (int j = 0; j < BTREE_ORDER - 1; j++) {
        newChild.records[j] = child.records[j + BTREE_ORDER];
    }

    // Copy children pointers if not leaf
    if (!child.isLeaf) {
        for (int j = 0; j < BTREE_ORDER; j++) {
            newChild.children[j] = child.children[j + BTREE_ORDER];
            // Clean up the moved pointers in the old child (optional but good for debugging)
            child.children[j + BTREE_ORDER] = INVALID_OFFSET;
        }
    }

    child.numKeys = BTREE_ORDER - 1;

    // Shift parent children to right
    for (int j = parent->numKeys; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = newChildOffset;

    // Shift parent keys to right
    for (int j = parent->numKeys - 1; j >= i; j--) {
        parent->records[j + 1] = parent->records[j];
    }

    // Move median key up to parent
    parent->records[i] = child.records[BTREE_ORDER - 1];
    parent->numKeys++;

    // Save changes
    write_node(f, childOffset, &child);
    write_node(f, newChildOffset, &newChild);
    write_node(f, parentOffset, parent);
}

// Requires the compare_keys function above
void btree_insert_non_full(FILE *f, long nodeOffset, IndexRecord k, BTreeHeader *header) {
    BTreeNode node;
    read_node(f, nodeOffset, &node);

    int i = node.numKeys - 1;

    if (node.isLeaf) {
        // FIXED: Use compare_keys instead of strcmp
        // Shift keys right to make space
        while (i >= 0 && compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) < 0) {
            node.records[i + 1] = node.records[i];
            i--;
        }
        
        // Insert struct
        node.records[i + 1] = k;
        node.numKeys++;
        write_node(f, nodeOffset, &node);
    } 
    else {
        // FIXED: Use compare_keys instead of strcmp
        // Find child index
        while (i >= 0 && compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) < 0) {
            i--;
        }
        i++;

        long childOffset = node.children[i];
        BTreeNode child;
        read_node(f, childOffset, &child);

        // If child is full, split it
        if (child.numKeys == 2 * BTREE_ORDER - 1) { // Ensure ORDER matches your header (ORDER vs BTREE_ORDER)
            btree_split_child(f, nodeOffset, &node, i, header);
            
            // FIXED: Use compare_keys to decide which half gets the new key
            if (compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
                i++;
            }
            childOffset = node.children[i];
        }
        btree_insert_non_full(f, childOffset, k, header);
    }
}

int btree_insert(FILE *treeFile, IndexRecord record) {
    BTreeHeader header;
    read_header(treeFile, &header);

    // Check Duplicate
    IndexRecord temp;
    if (btree_search(treeFile, record.recordName, record.thresholdValue, &temp)) {
        return 0; 
    }

    BTreeNode root;
    read_node(treeFile, header.rootNodeOffset, &root);

    // If root is full, tree grows in height
    if (root.numKeys == 2 * BTREE_ORDER - 1) { // Fixed typo here too
        long newRootOffset = get_new_node_offset(treeFile, &header);
        BTreeNode newRoot;
        memset(&newRoot, 0, sizeof(BTreeNode));
        
        // --- FIX START: INITIALIZE POINTERS TO -1 ---
        for(int k=0; k < 2*BTREE_ORDER; k++) {
            newRoot.children[k] = INVALID_OFFSET;
        }
        // --- FIX END ---
        
        newRoot.isLeaf = 0;
        newRoot.numKeys = 0;
        newRoot.children[0] = header.rootNodeOffset;

        write_node(treeFile, newRootOffset, &newRoot);

        // Update header
        header.rootNodeOffset = newRootOffset;
        write_header(treeFile, &header);

        // Split the old root
        btree_split_child(treeFile, newRootOffset, &newRoot, 0, &header);

        // Insert
        btree_insert_non_full(treeFile, newRootOffset, record, &header);
    } else {
        btree_insert_non_full(treeFile, header.rootNodeOffset, record, &header);
    }
    return 1;
}

// Add this helper function
int validate_btree_file(FILE *treeFile) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    printf("Validando arquivo B-Tree...\n");
    printf("Root offset: %ld\n", header.rootNodeOffset);
    printf("Next free offset: %ld\n", header.nextFreeOffset);
    
    // Check if root offset makes sense
    if (header.rootNodeOffset < sizeof(BTreeHeader)) {
        printf("[ERRO] Root offset invalido!\n");
        return 0;
    }
    
    // Try to read root
    BTreeNode root;
    fseek(treeFile, header.rootNodeOffset, SEEK_SET);
    if (fread(&root, sizeof(BTreeNode), 1, treeFile) != 1) {
        printf("[ERRO] Nao foi possivel ler no raiz!\n");
        return 0;
    }
    
    if (root.numKeys < 0 || root.numKeys > 2 * BTREE_ORDER) {
        printf("[ERRO] No raiz corrompido: numKeys = %d\n", root.numKeys);
        return 0;
    }
    
    printf("Arquivo parece valido.\n");
    return 1;
}

void debug_btree_structure(FILE *treeFile) {
    printf("\n=== DIAGNÓSTICO DA B-TREE ===\n");
    
    // 1. Ler header
    BTreeHeader header;
    read_header(treeFile, &header);
    printf("Header:\n");
    printf("  Root offset: %ld\n", header.rootNodeOffset);
    printf("  Next free offset: %ld\n", header.nextFreeOffset);
    printf("  Tamanho esperado do header: %zu\n", sizeof(BTreeHeader));
    printf("  Tamanho esperado do nó: %zu\n", sizeof(BTreeNode));
    
    // 2. Verificar se o root offset faz sentido
    long expectedRootOffset = sizeof(BTreeHeader);
    printf("  Root offset esperado: %ld\n", expectedRootOffset);
    
    if (header.rootNodeOffset != expectedRootOffset) {
        printf("  ❌ PROBLEMA: Root offset incorreto!\n");
    }
    
    // 3. Tentar ler o nó raiz
    printf("\nTentando ler nó raiz em offset %ld:\n", header.rootNodeOffset);
    
    BTreeNode root;
    fseek(treeFile, header.rootNodeOffset, SEEK_SET);
    if (fread(&root, sizeof(BTreeNode), 1, treeFile) != 1) {
        printf("  ❌ Falha ao ler nó raiz!\n");
        return;
    }
    
    printf("  numKeys: %d\n", root.numKeys);
    printf("  isLeaf: %d\n", root.isLeaf);
    
    // 4. Se numKeys parece válido, mostrar os dados
    if (root.numKeys >= 0 && root.numKeys <= 2 * BTREE_ORDER) {
        printf("  ✅ Nó raiz parece válido\n");
        
        for (int i = 0; i < root.numKeys; i++) {
            printf("    Registro %d: %s (limiar %d, deleted=%d)\n", 
                   i, root.records[i].recordName, 
                   root.records[i].thresholdValue,
                   root.records[i].isDeleted);
            printf("      dataOffset: %ld, blockSize: %ld\n",
                   root.records[i].dataOffset, root.records[i].blockSize);
        }
        
        if (!root.isLeaf) {
            printf("    Filhos:\n");
            for (int i = 0; i <= root.numKeys; i++) {
                printf("      children[%d]: %ld\n", i, root.children[i]);
                
                // ⚠️ AQUI ESTÁ O PROBLEMA PROVÁVEL!
                if (root.children[i] != INVALID_OFFSET && root.children[i] < 1000) {
                    printf("        ❌ SUSPEITO: Offset muito pequeno (%ld)!\n", root.children[i]);
                }
            }
        }
    } else {
        printf("  ❌ Nó raiz corrompido!\n");
        
        // Mostrar os primeiros bytes como hex para debug
        printf("  Primeiros 32 bytes do nó:\n  ");
        unsigned char *nodeBytes = (unsigned char*)&root;
        for (int i = 0; i < 32 && i < sizeof(BTreeNode); i++) {
            printf("%02X ", nodeBytes[i]);
            if ((i + 1) % 16 == 0) printf("\n  ");
        }
        printf("\n");
    }
    
    printf("===============================\n\n");
}



void _collect_from_node(FILE *treeFile, long nodeOffset, IndexRecord **records, int *count, int *capacity) {
    if (nodeOffset == INVALID_OFFSET) return;
    
    // Safety checks
    if (nodeOffset < sizeof(BTreeHeader)) return;
    
    BTreeNode node;
    fseek(treeFile, nodeOffset, SEEK_SET);
    if (fread(&node, sizeof(BTreeNode), 1, treeFile) != 1) return;
    
    // Validate node
    if (node.numKeys < 0 || node.numKeys > 2 * BTREE_ORDER) {
        printf("Aviso: No corrompido encontrado (offset %ld), pulando...\n", nodeOffset);
        return;
    }

    // Traverse in-order
    for (int i = 0; i < node.numKeys; i++) {
        // Process left child first
        if (!node.isLeaf) {
            _collect_from_node(treeFile, node.children[i], records, count, capacity);
        }
        
        // Process current record
        if (node.records[i].isDeleted == 0) { // Not deleted
            // Expand array if needed
            if (*count >= *capacity) {
                *capacity *= 2;
                *records = realloc(*records, *capacity * sizeof(IndexRecord));
                if (!*records) {
                    printf("Erro de memoria durante coleta.\n");
                    return;
                }
            }
            
            // Add to collection
            (*records)[*count] = node.records[i];
            (*count)++;
        }
    }
    
    // Process rightmost child
    if (!node.isLeaf) {
        _collect_from_node(treeFile, node.children[node.numKeys], records, count, capacity);
    }
}

void _btree_print_recursive(FILE *f, long nodeOffset) {
    if (nodeOffset == INVALID_OFFSET) return;

    BTreeNode node;
    read_node(f, nodeOffset, &node);

    int i;
    for (i = 0; i < node.numKeys; i++) {
        // 1. Go Left
        if (!node.isLeaf) _btree_print_recursive(f, node.children[i]);
        
        // 2. Print Current Key
        if(!node.records[i].isDeleted){
            printf("Imagem: %-20s | Limiar: %3d | Offset: %ld | Size: %ld\n", 
                node.records[i].recordName, 
                node.records[i].thresholdValue,
                node.records[i].dataOffset,
                node.records[i].blockSize);
        }
    }

    // 3. Go Right (Last child)
    if (!node.isLeaf) _btree_print_recursive(f, node.children[i]);
}

int btree_delete(FILE* treeFile, const char* key, int threshold) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    long currentOffset = header.rootNodeOffset;
    BTreeNode node;

    // Traverse the tree just like Search
    while (currentOffset != INVALID_OFFSET) {
        read_node(treeFile, currentOffset, &node);
        
        int i = 0;
        // Find the first key greater than or equal to target
        while (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
            i++;
        }

        // Check if we found the EXACT match
        if (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) == 0) {
            
            // --- THE CHANGE ---
            if (node.records[i].isDeleted == 1) {
                return 0; // Already deleted
            }

            // 1. Mark as deleted
            node.records[i].isDeleted = 1;

            // 2. Write the modified node BACK to the same offset
            write_node(treeFile, currentOffset, &node);
            
            return 1; // Success
        }

        // Not found in this node...
        if (node.isLeaf) {
            return 0; // Reached bottom, record doesn't exist
        }

        // Go deeper
        currentOffset = node.children[i];
    }
    return 0;
}



// Helper function to collect all valid records from B-tree
IndexRecord* btree_collect_all_records(FILE *treeFile, int *totalRecords) {
    *totalRecords = 0;
    int capacity = 100; // Initial capacity
    IndexRecord *records = malloc(capacity * sizeof(IndexRecord));
    if (!records) {
        printf("Erro: Sem memoria para coletar registros.\n");
        return NULL;
    }
    
    BTreeHeader header;
    read_header(treeFile, &header);
    
    // Use the existing helper function to collect records in-order
    _collect_from_node(treeFile, header.rootNodeOffset, &records, totalRecords, &capacity);
    
    printf("Coletados %d registros validos da B-tree.\n", *totalRecords);
    return records;
}

int btree_compact_database(const char *binaryDataFile, const char *btreeIndexFile) {
    printf("Iniciando compactacao do banco de dados...\n");
    
    // 1. Open original files
    FILE *binInStream = fopen(binaryDataFile, "rb");
    if (!binInStream) {
        perror("Erro ao abrir arquivo de dados original");
        return -1;
    }
    
    FILE *btreeInStream = btree_open(btreeIndexFile);
    if (!btreeInStream) {
        fclose(binInStream);
        perror("Erro ao abrir B-tree original");
        return -1;
    }
    
    // 2. Create temporary files
    FILE *tempDataStream = fopen(TEMP_DATABASE, "wb");
    if (!tempDataStream) {
        fclose(binInStream);
        btree_close(btreeInStream);
        perror("Erro ao criar arquivo de dados temporario");
        return -1;
    }
    
    FILE *tempBtreeStream = btree_open(TEMP_INDEX_BTREE);
    if (!tempBtreeStream) {
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        perror("Erro ao criar B-tree temporaria");
        return -1;
    }
    
    // 3. Collect all valid records from original B-tree
    int totalRecords = 0;
    IndexRecord *validRecords = btree_collect_all_records(btreeInStream, &totalRecords);
    if (!validRecords) {
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        btree_close(tempBtreeStream);
        return -1;
    }
    
    // 4. Copy data and rebuild index
    long newOffset = 0;
    int recordsCopied = 0;
    const size_t BUF_SZ = 8192;
    char *copyBuffer = malloc(BUF_SZ);
    
    if (!copyBuffer) {
        perror("Sem memoria para buffer de copia");
        free(validRecords);
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        btree_close(tempBtreeStream);
        return -1;
    }
    
    printf("Copiando %d registros validos...\n", totalRecords);
    
    for (int i = 0; i < totalRecords; i++) {
        IndexRecord currentRecord = validRecords[i];
        
        // Skip if somehow a deleted record got through
        if (currentRecord.isDeleted == 1) {
            continue;
        }
        
        printf("Copiando: %s (limiar %d) - %ld bytes\n", 
               currentRecord.recordName, 
               currentRecord.thresholdValue, 
               currentRecord.blockSize);
        
        // Seek to original position in data file
        fseek(binInStream, currentRecord.dataOffset, SEEK_SET);
        
        long remainingBytes = currentRecord.blockSize;
        
        // Copy data in chunks
        while (remainingBytes > 0) {
            size_t bytesToRead = (remainingBytes > (long)BUF_SZ) ? BUF_SZ : (size_t)remainingBytes;
            size_t bytesRead = fread(copyBuffer, 1, bytesToRead, binInStream);
            
            if (bytesRead == 0) {
                printf("Aviso: Falha na leitura do registro %s\n", currentRecord.recordName);
                break;
            }
            
            fwrite(copyBuffer, 1, bytesRead, tempDataStream);
            remainingBytes -= bytesRead;
        }
        
        // Create new record with updated offset
        IndexRecord newRecord = currentRecord;
        newRecord.dataOffset = newOffset;
        newRecord.isDeleted = 0; // Ensure it's marked as valid
        
        // Insert into new B-tree
        if (btree_insert(tempBtreeStream, newRecord)) {
            recordsCopied++;
            newOffset += currentRecord.blockSize;
        } else {
            printf("Aviso: Falha ao inserir %s na nova B-tree\n", currentRecord.recordName);
        }
    }
    
    // 5. Cleanup
    free(copyBuffer);
    free(validRecords);
    fclose(binInStream);
    btree_close(btreeInStream);
    fclose(tempDataStream);
    btree_close(tempBtreeStream);
    
    // 6. Replace original files with compacted versions
    printf("Substituindo arquivos originais...\n");
    
    if (remove(binaryDataFile) != 0) {
        perror("Aviso: nao foi possivel remover arquivo de dados antigo");
    }
    if (rename(TEMP_DATABASE, binaryDataFile) != 0) {
        perror("Erro critico: nao foi possivel renomear arquivo de dados");
        return -1;
    }
    
    if (remove(btreeIndexFile) != 0) {
        perror("Aviso: nao foi possivel remover B-tree antiga");
    }
    if (rename(TEMP_INDEX_BTREE, btreeIndexFile) != 0) {
        perror("Erro critico: nao foi possivel renomear B-tree");
        return -1;
    }
    
    printf("Compactacao concluida com sucesso!\n");
    printf("Registros copiados: %d\n", recordsCopied);
    printf("Espaco liberado: registros deletados foram removidos.\n");
    
    return recordsCopied;
}

// Public function called by main
void btree_print_all(FILE *treeFile) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    printf("\n=== IMAGENS NO INDICE (B-Tree) ===\n");
    _btree_print_recursive(treeFile, header.rootNodeOffset);
    printf("==================================\n");
}