#include "btree.h"
#include <stdlib.h>
#include <string.h>

// constantes para arquivos temporários durante compactação
#define TEMP_DATABASE "temp_data.bin"
#define TEMP_INDEX_BTREE "temp_index.btree"

// === funções auxiliares para comparação ===

// compara duas chaves (nome + limiar) - usado internamente pela b-tree
int compare_keys(const char* name1, int thresh1, const char* name2, int thresh2) {
    // primeiro compara os nomes
    int nameCmp = strcmp(name1, name2);
    if (nameCmp != 0) return nameCmp;
    
    // se nomes iguais, compara limiares
    if (thresh1 < thresh2) return -1;
    if (thresh1 > thresh2) return 1;
    
    return 0; // chaves idênticas
}

// compara uma chave de busca com um registro existente
int compare_records(const char* keyName, int keyThreshold, IndexRecord existingRecord) {
    // 1. compara nomes primeiro
    int nameCmp = strncmp(keyName, existingRecord.recordName, 50);
    
    // 2. se nomes diferentes, retorna resultado
    if (nameCmp != 0) {
        return nameCmp;
    }
    
    // 3. se nomes iguais, compara limiares
    // isso permite "img.pgm" (100) e "img.pgm" (150) existirem juntos
    if (keyThreshold < existingRecord.thresholdValue) return -1;
    if (keyThreshold > existingRecord.thresholdValue) return 1;
    
    return 0; // registro exatamente igual
}

// === funções de leitura/escrita de nós ===

// lê um nó do arquivo na posição especificada
void read_node(FILE *f, long offset, BTreeNode *node) {
    if (offset == INVALID_OFFSET) return;
    fseek(f, offset, SEEK_SET);
    fread(node, sizeof(BTreeNode), 1, f);
}

// escreve um nó no arquivo na posição especificada
void write_node(FILE *f, long offset, BTreeNode *node) {
    fseek(f, offset, SEEK_SET);
    fwrite(node, sizeof(BTreeNode), 1, f);
}

// lê o cabeçalho do arquivo (sempre no início)
void read_header(FILE *f, BTreeHeader *header) {
    fseek(f, 0, SEEK_SET);
    fread(header, sizeof(BTreeHeader), 1, f);
}

// escreve o cabeçalho no arquivo (sempre no início)
void write_header(FILE *f, BTreeHeader *header) {
    fseek(f, 0, SEEK_SET);
    fwrite(header, sizeof(BTreeHeader), 1, f);
}

// obtém um novo offset para escrever um nó (sempre no final do arquivo)
long get_new_node_offset(FILE *f, BTreeHeader *header) {
    long offset = header->nextFreeOffset;
    header->nextFreeOffset += sizeof(BTreeNode);
    write_header(f, header); 
    return offset;
}

// === funções de gerenciamento de arquivos ===

// abre ou cria um arquivo de b-tree
FILE* btree_open(const char* filename) {
    // tenta abrir arquivo existente
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        // se não existe, cria novo
        f = fopen(filename, "wb+");
        if (!f) return NULL;

        // inicializa cabeçalho
        BTreeHeader header;
        header.rootNodeOffset = sizeof(BTreeHeader);
        header.nextFreeOffset = header.rootNodeOffset + sizeof(BTreeNode);
        write_header(f, &header);

        // cria nó raiz vazio
        BTreeNode root;
        memset(&root, 0, sizeof(BTreeNode)); // zera tudo
        
        // importante: define todos os filhos como inválidos
        for(int i=0; i < 2*BTREE_ORDER; i++) {
            root.children[i] = INVALID_OFFSET; 
        }

        root.isLeaf = 1;    // raiz começa como folha
        root.numKeys = 0;   // sem chaves inicialmente
        write_node(f, header.rootNodeOffset, &root);
    }
    return f;
}

// fecha o arquivo da b-tree
void btree_close(FILE* treeFile) {
    if (treeFile) fclose(treeFile);
}

// === lógica de busca ===

// procura por uma chave específica na b-tree
int btree_search(FILE* treeFile, const char* key, int threshold, IndexRecord* foundRecord) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    long currentOffset = header.rootNodeOffset;
    BTreeNode node;

    // percorre a árvore de cima para baixo
    while (currentOffset != INVALID_OFFSET) {
        read_node(treeFile, currentOffset, &node);
        
        int i = 0;
        // encontra a primeira chave maior que a procurada
        while (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
            i++;
        }

        // verifica se encontrou exatamente (nome == nome AND limiar == limiar)
        if (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) == 0) {
            if (foundRecord) *foundRecord = node.records[i];
            return 1; // encontrou correspondência exata
        }

        if (node.isLeaf) return 0; // chegou na folha, não encontrou
        currentOffset = node.children[i]; // desce para o filho apropriado
    }
    return 0;
}

// === lógica de inserção (com divisão de nós) ===

// divide um nó filho que está cheio
void btree_split_child(FILE *f, long parentOffset, BTreeNode *parent, int i, BTreeHeader *header) {
    long childOffset = parent->children[i];
    BTreeNode child;
    read_node(f, childOffset, &child);

    // cria novo nó (metade direita)
    long newChildOffset = get_new_node_offset(f, header);
    BTreeNode newChild;
    memset(&newChild, 0, sizeof(BTreeNode));
    
    // inicializa ponteiros como inválidos
    for(int k=0; k < 2*BTREE_ORDER; k++) {
        newChild.children[k] = INVALID_OFFSET;
    }

    newChild.isLeaf = child.isLeaf;
    newChild.numKeys = BTREE_ORDER - 1;

    // copia metade superior das chaves para o novo nó
    for (int j = 0; j < BTREE_ORDER - 1; j++) {
        newChild.records[j] = child.records[j + BTREE_ORDER];
    }

    // copia ponteiros dos filhos se não for folha
    if (!child.isLeaf) {
        for (int j = 0; j < BTREE_ORDER; j++) {
            newChild.children[j] = child.children[j + BTREE_ORDER];
            // limpa os ponteiros movidos no nó antigo
            child.children[j + BTREE_ORDER] = INVALID_OFFSET;
        }
    }

    child.numKeys = BTREE_ORDER - 1;

    // desloca filhos do pai para a direita
    for (int j = parent->numKeys; j >= i + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[i + 1] = newChildOffset;

    // desloca chaves do pai para a direita
    for (int j = parent->numKeys - 1; j >= i; j--) {
        parent->records[j + 1] = parent->records[j];
    }

    // move chave mediana para o pai
    parent->records[i] = child.records[BTREE_ORDER - 1];
    parent->numKeys++;

    // salva mudanças
    write_node(f, childOffset, &child);
    write_node(f, newChildOffset, &newChild);
    write_node(f, parentOffset, parent);
}

// insere em um nó que não está cheio
void btree_insert_non_full(FILE *f, long nodeOffset, IndexRecord k, BTreeHeader *header) {
    BTreeNode node;
    read_node(f, nodeOffset, &node);

    int i = node.numKeys - 1;

    if (node.isLeaf) {
        // desloca chaves para a direita para abrir espaço
        while (i >= 0 && compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) < 0) {
            node.records[i + 1] = node.records[i];
            i--;
        }
        
        // insere o novo registro
        node.records[i + 1] = k;
        node.numKeys++;
        write_node(f, nodeOffset, &node);
    } 
    else {
        // encontra índice do filho correto
        while (i >= 0 && compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) < 0) {
            i--;
        }
        i++;

        long childOffset = node.children[i];
        BTreeNode child;
        read_node(f, childOffset, &child);

        // se filho está cheio, divide ele
        if (child.numKeys == 2 * BTREE_ORDER - 1) {
            btree_split_child(f, nodeOffset, &node, i, header);
            
            // decide qual metade recebe a nova chave
            if (compare_keys(k.recordName, k.thresholdValue, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
                i++;
            }
            childOffset = node.children[i];
        }
        btree_insert_non_full(f, childOffset, k, header);
    }
}

// função principal de inserção
int btree_insert(FILE *treeFile, IndexRecord record) {
    BTreeHeader header;
    read_header(treeFile, &header);

    // verifica duplicata
    IndexRecord temp;
    if (btree_search(treeFile, record.recordName, record.thresholdValue, &temp)) {
        return 0; // já existe
    }

    BTreeNode root;
    read_node(treeFile, header.rootNodeOffset, &root);

    // se raiz está cheia, árvore cresce em altura
    if (root.numKeys == 2 * BTREE_ORDER - 1) {
        long newRootOffset = get_new_node_offset(treeFile, &header);
        BTreeNode newRoot;
        memset(&newRoot, 0, sizeof(BTreeNode));
        
        // inicializa ponteiros como inválidos
        for(int k=0; k < 2*BTREE_ORDER; k++) {
            newRoot.children[k] = INVALID_OFFSET;
        }
        
        newRoot.isLeaf = 0;
        newRoot.numKeys = 0;
        newRoot.children[0] = header.rootNodeOffset;

        write_node(treeFile, newRootOffset, &newRoot);

        // atualiza cabeçalho
        header.rootNodeOffset = newRootOffset;
        write_header(treeFile, &header);

        // divide a raiz antiga
        btree_split_child(treeFile, newRootOffset, &newRoot, 0, &header);

        // insere
        btree_insert_non_full(treeFile, newRootOffset, record, &header);
    } else {
        btree_insert_non_full(treeFile, header.rootNodeOffset, record, &header);
    }
    return 1;
}

// === funções de debug e validação ===

// valida se o arquivo da b-tree está íntegro
int validate_btree_file(FILE *treeFile) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    printf("validando arquivo b-tree...\n");
    printf("root offset: %ld\n", header.rootNodeOffset);
    printf("next free offset: %ld\n", header.nextFreeOffset);
    
    // verifica se offset da raiz faz sentido
    if (header.rootNodeOffset < sizeof(BTreeHeader)) {
        printf("[erro] root offset invalido!\n");
        return 0;
    }
    
    // tenta ler raiz
    BTreeNode root;
    fseek(treeFile, header.rootNodeOffset, SEEK_SET);
    if (fread(&root, sizeof(BTreeNode), 1, treeFile) != 1) {
        printf("[erro] nao foi possivel ler no raiz!\n");
        return 0;
    }
    
    if (root.numKeys < 0 || root.numKeys > 2 * BTREE_ORDER) {
        printf("[erro] no raiz corrompido: numkeys = %d\n", root.numKeys);
        return 0;
    }
    
    printf("arquivo parece valido.\n");
    return 1;
}

// mostra estrutura detalhada da b-tree para debug
void debug_btree_structure(FILE *treeFile) {
    printf("\n=== diagnóstico da b-tree ===\n");
    
    // ler cabeçalho
    BTreeHeader header;
    read_header(treeFile, &header);
    printf("header:\n");
    printf("  root offset: %ld\n", header.rootNodeOffset);
    printf("  next free offset: %ld\n", header.nextFreeOffset);
    printf("  tamanho esperado do header: %zu\n", sizeof(BTreeHeader));
    printf("  tamanho esperado do nó: %zu\n", sizeof(BTreeNode));
    
    // verificar se o root offset faz sentido
    long expectedRootOffset = sizeof(BTreeHeader);
    printf("root offset esperado: %ld\n", expectedRootOffset);
    
    if (header.rootNodeOffset != expectedRootOffset) {
        printf("  ❌ problema: root offset incorreto!\n");
    }
    
    // tentar ler o nó raiz
    printf("\ntentando ler nó raiz em offset %ld:\n", header.rootNodeOffset);
    
    BTreeNode root;
    fseek(treeFile, header.rootNodeOffset, SEEK_SET);
    if (fread(&root, sizeof(BTreeNode), 1, treeFile) != 1) {
        printf("falha ao ler nó raiz!\n");
        return;
    }
    
    printf("numkeys: %d\n", root.numKeys);
    printf("isleaf: %d\n", root.isLeaf);
    
    // se numkeys parece válido, mostrar os dados
    if (root.numKeys >= 0 && root.numKeys <= 2 * BTREE_ORDER) {
        printf("nó raiz parece válido\n");
        
        for (int i = 0; i < root.numKeys; i++) {
            printf("registro %d: %s (limiar %d, deleted=%d)\n", 
                   i, root.records[i].recordName, 
                   root.records[i].thresholdValue,
                   root.records[i].isDeleted);
            printf("dataoffset: %ld, blocksize: %ld\n",
                   root.records[i].dataOffset, root.records[i].blockSize);
        }
        
        if (!root.isLeaf) {
            printf(" filhos:\n");
            for (int i = 0; i <= root.numKeys; i++) {
                printf(" children[%d]: %ld\n", i, root.children[i]);
                
                if (root.children[i] != INVALID_OFFSET && root.children[i] < 1000) {
                    printf(" suspeito: offset muito pequeno (%ld)!\n", root.children[i]);
                }
            }
        }
    } else {
        printf(" nó raiz corrompido!\n");
        
        // mostra os primeiros bytes como hex para debug
        printf("  primeiros 32 bytes do nó:\n  ");
        unsigned char *nodeBytes = (unsigned char*)&root;
        for (int i = 0; i < 32 && i < sizeof(BTreeNode); i++) {
            printf("%02x ", nodeBytes[i]);
            if ((i + 1) % 16 == 0) printf("\n  ");
        }
        printf("\n");
    }
    
    printf("===============================\n\n");
}

// === funções de coleta e travessia ===

// coleta recursivamente todos os registros válidos de um nó
void _collect_from_node(FILE *treeFile, long nodeOffset, IndexRecord **records, int *count, int *capacity) {
    if (nodeOffset == INVALID_OFFSET) return;
    
    // verificações de segurança
    if (nodeOffset < sizeof(BTreeHeader)) return;
    
    BTreeNode node;
    fseek(treeFile, nodeOffset, SEEK_SET);
    if (fread(&node, sizeof(BTreeNode), 1, treeFile) != 1) return;
    
    // valida nó
    if (node.numKeys < 0 || node.numKeys > 2 * BTREE_ORDER) {
        printf("aviso: nó corrompido encontrado (offset %ld), pulando...\n", nodeOffset);
        return;
    }

    // percorre em ordem (in-order traversal)
    for (int i = 0; i < node.numKeys; i++) {
        // processa filho esquerdo primeiro
        if (!node.isLeaf) {
            _collect_from_node(treeFile, node.children[i], records, count, capacity);
        }
        
        // processa registro atual
        if (node.records[i].isDeleted == 0) { // não deletado
            // expande array se necessário
            if (*count >= *capacity) {
                *capacity *= 2;
                *records = realloc(*records, *capacity * sizeof(IndexRecord));
                if (!*records) {
                    printf("erro de memoria durante coleta.\n");
                    return;
                }
            }
            
            // adiciona à coleção
            (*records)[*count] = node.records[i];
            (*count)++;
        }
    }
    
    // processa filho mais à direita
    if (!node.isLeaf) {
        _collect_from_node(treeFile, node.children[node.numKeys], records, count, capacity);
    }
}

// imprime recursivamente toda a árvore
void _btree_print_recursive(FILE *f, long nodeOffset) {
    if (nodeOffset == INVALID_OFFSET) return;

    BTreeNode node;
    read_node(f, nodeOffset, &node);

    int i;
    for (i = 0; i < node.numKeys; i++) {
        // vai para esquerda
        if (!node.isLeaf) _btree_print_recursive(f, node.children[i]);
        
        // imprime chave atual
        if(!node.records[i].isDeleted){
            printf("imagem: %-20s | limiar: %3d | offset: %ld | size: %ld\n", 
                node.records[i].recordName, 
                node.records[i].thresholdValue,
                node.records[i].dataOffset,
                node.records[i].blockSize);
        }
    }

    // vai para direita (último filho)
    if (!node.isLeaf) _btree_print_recursive(f, node.children[i]);
}

// === funções de deleção e compactação ===

// marca um registro como deletado (deleção preguiçosa)
int btree_delete(FILE* treeFile, const char* key, int threshold) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    long currentOffset = header.rootNodeOffset;
    BTreeNode node;

    // percorre a árvore como na busca
    while (currentOffset != INVALID_OFFSET) {
        read_node(treeFile, currentOffset, &node);
        
        int i = 0;
        // encontra a primeira chave maior ou igual ao alvo
        while (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) > 0) {
            i++;
        }

        // verifica se encontrou correspondência exata
        if (i < node.numKeys && compare_keys(key, threshold, node.records[i].recordName, node.records[i].thresholdValue) == 0) {
            
            if (node.records[i].isDeleted == 1) {
                return 0; // já deletado
            }

            // marca como deletado
            node.records[i].isDeleted = 1;

            // escreve o nó modificado de volta no mesmo offset
            write_node(treeFile, currentOffset, &node);
            
            return 1; // sucesso
        }

        // não encontrado neste nó...
        if (node.isLeaf) {
            return 0; // chegou no fundo, registro não existe
        }

        // vai mais fundo
        currentOffset = node.children[i];
    }
    return 0;
}

// coleta todos os registros válidos da b-tree
IndexRecord* btree_collect_all_records(FILE *treeFile, int *totalRecords) {
    *totalRecords = 0;
    int capacity = 100; // capacidade inicial
    IndexRecord *records = malloc(capacity * sizeof(IndexRecord));
    if (!records) {
        printf("erro: sem memoria para coletar registros.\n");
        return NULL;
    }
    
    BTreeHeader header;
    read_header(treeFile, &header);
    
    // usa a função auxiliar existente para coletar registros em ordem
    _collect_from_node(treeFile, header.rootNodeOffset, &records, totalRecords, &capacity);
    
    printf("coletados %d registros validos da b-tree.\n", *totalRecords);
    return records;
}

// compacta o banco de dados removendo registros deletados
int btree_compact_database(const char *binaryDataFile, const char *btreeIndexFile) {
    printf("iniciando compactacao do banco de dados...\n");
    
    // abre arquivos originais
    FILE *binInStream = fopen(binaryDataFile, "rb");
    if (!binInStream) {
        perror("erro ao abrir arquivo de dados original");
        return -1;
    }
    
    FILE *btreeInStream = btree_open(btreeIndexFile);
    if (!btreeInStream) {
        fclose(binInStream);
        perror("erro ao abrir b-tree original");
        return -1;
    }
    
    // cria arquivos temporários
    FILE *tempDataStream = fopen(TEMP_DATABASE, "wb");
    if (!tempDataStream) {
        fclose(binInStream);
        btree_close(btreeInStream);
        perror("erro ao criar arquivo de dados temporario");
        return -1;
    }
    
    FILE *tempBtreeStream = btree_open(TEMP_INDEX_BTREE);
    if (!tempBtreeStream) {
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        perror("erro ao criar b-tree temporaria");
        return -1;
    }
    
    // coleta todos os registros válidos da b-tree original
    int totalRecords = 0;
    IndexRecord *validRecords = btree_collect_all_records(btreeInStream, &totalRecords);
    if (!validRecords) {
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        btree_close(tempBtreeStream);
        return -1;
    }
    
    // copia dados e reconstrói índice
    long newOffset = 0;
    int recordsCopied = 0;
    const size_t BUF_SZ = 8192;
    char *copyBuffer = malloc(BUF_SZ);
    
    if (!copyBuffer) {
        perror("sem memoria para buffer de copia");
        free(validRecords);
        fclose(binInStream);
        btree_close(btreeInStream);
        fclose(tempDataStream);
        btree_close(tempBtreeStream);
        return -1;
    }
    
    printf("copiando %d registros validos...\n", totalRecords);
    
    for (int i = 0; i < totalRecords; i++) {
        IndexRecord currentRecord = validRecords[i];
        
        // pula se de alguma forma um registro deletado passou
        if (currentRecord.isDeleted == 1) {
            continue;
        }
        
        printf("copiando: %s (limiar %d) - %ld bytes\n", 
               currentRecord.recordName, 
               currentRecord.thresholdValue, 
               currentRecord.blockSize);
        
        // busca posição original no arquivo de dados
        fseek(binInStream, currentRecord.dataOffset, SEEK_SET);
        
        long remainingBytes = currentRecord.blockSize;
        
        // copia dados em pedaços
        while (remainingBytes > 0) {
            size_t bytesToRead = (remainingBytes > (long)BUF_SZ) ? BUF_SZ : (size_t)remainingBytes;
            size_t bytesRead = fread(copyBuffer, 1, bytesToRead, binInStream);
            
            if (bytesRead == 0) {
                printf("aviso: falha na leitura do registro %s\n", currentRecord.recordName);
                break;
            }
            
            fwrite(copyBuffer, 1, bytesRead, tempDataStream);
            remainingBytes -= bytesRead;
        }
        
        // cria novo registro com offset atualizado
        IndexRecord newRecord = currentRecord;
        newRecord.dataOffset = newOffset;
        newRecord.isDeleted = 0; // garante que está marcado como válido
        
        // insere na nova b-tree
        if (btree_insert(tempBtreeStream, newRecord)) {
            recordsCopied++;
            newOffset += currentRecord.blockSize;
        } else {
            printf("aviso: falha ao inserir %s na nova b-tree\n", currentRecord.recordName);
        }
    }
    
    // limpeza
    free(copyBuffer);
    free(validRecords);
    fclose(binInStream);
    btree_close(btreeInStream);
    fclose(tempDataStream);
    btree_close(tempBtreeStream);
    
    // substitui arquivos originais pelas versões compactadas
    printf("substituindo arquivos originais...\n");
    
    if (remove(binaryDataFile) != 0) {
        perror("aviso: nao foi possivel remover arquivo de dados antigo");
    }
    if (rename(TEMP_DATABASE, binaryDataFile) != 0) {
        perror("erro critico: nao foi possivel renomear arquivo de dados");
        return -1;
    }
    
    if (remove(btreeIndexFile) != 0) {
        perror("aviso: nao foi possivel remover b-tree antiga");
    }
    if (rename(TEMP_INDEX_BTREE, btreeIndexFile) != 0) {
        perror("erro critico: nao foi possivel renomear b-tree");
        return -1;
    }
    
    printf("compactacao concluida com sucesso!\n");
    printf("registros copiados: %d\n", recordsCopied);
    printf("espaco liberado: registros deletados foram removidos.\n");
    
    return recordsCopied;
}

// função pública chamada pelo main - imprime toda a árvore
void btree_print_all(FILE *treeFile) {
    BTreeHeader header;
    read_header(treeFile, &header);
    
    printf("\n=== imagens no indice (b-tree) ===\n");
    _btree_print_recursive(treeFile, header.rootNodeOffset);
    printf("==================================\n");
}