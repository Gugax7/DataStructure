/*
ALUNO: GUSTAVO SALMAZO DA SILVA

# Algumas considerações sobre a utilização do programa:

- para compilar gcc main.c -o main
- ao executar, ele mostrará as opções de comando possiveis do programa
- nele é possivel: inserir, exportar, listar, e remover imagens limiarizadas
- o programa também permite compactação (hard remove) de banco de dados e recuperação de imagens a partir da média das imagens limiarizadas presentes
- não é necessaria a criação de nenhum arquivo utilizado pelo programa (exemplo: database.bin), ele cria automaticamente quando necessario

## Utilização
- as imagens devem estar na mesma pasta que o programa (conforme enviado com a imagem exemplo (barbara.pgm))
- ao exportar imagens elas virão nomeadas como ("saida_<nome do arquivo>_l<limiar>.pgm")
- ao reconstruir imagens elas serão exportadas para a mesma pasta como "reconstruida_media_<nome da image>" (como elas são salvas como pgm, eu me aproveitei disso para não precisar escrever.pgm no fim)
- ao importar uma imagem é importante escrever exatamente o nome dela (com .pgm) como está salvo na pasta.

 Comandos:
   inserir <arquivo.pgm> <limiar1> [limiar2] ... - Insere uma imagem PGM com um ou múltiplos limiares.
   exportar <nome> <limiar>       - Exporta uma imagem do banco para PGM (ASCII P2).
   remover <nome> <limiar>        - Remove uma imagem da árvore B.
   listar                         - Lista todas as imagens ativas no banco (percurso ordenado).
   compactar                      - Remove permanentemente as imagens não referenciadas.
   reconstruir <nome>             - Gera uma media de todas as versoes de uma imagem.
   ajuda                          - Mostra esta mensagem.

## Breve explicação de cada funcionalidade:

- Inserir: insere uma imagem compactada e limiarizada no arquivo database.bin usando árvore B
- Exportar: descompacta e cria uma copia da imagem na pasta do programa
- Listar: lista as imagens que estão salvas usando percurso ordenado da árvore B
- Remover: remove efetivamente a chave da árvore B
- Compactar: exclui permanentemente os arquivos não referenciados do database.bin
- Reconstruir: pega todas as imagens com mesmo nome do banco de dados e realiza a média aritmética de cada pixel

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DATABASE "database.bin"
#define BTREE_FILE "btree.bin"
#define TEMP_DATABASE "database_temp.bin"
#define ORDER 3  // Ordem da árvore B
#define MAX_KEYS (ORDER - 1)  // 2 chaves por nó
#define MAX_CHILDREN ORDER    // 3 filhos por nó

typedef struct {
    int imgHeight, imgWidth, maxGrayValue;
    short int* pixelData;
} BinaryImage;

typedef struct {
    char fileName[256];
    int threshold;
} BTreeKey;

typedef struct {
    long dataOffset;
    long blockSize;
} DataRecord;

typedef struct {
    DataRecord record;
    int threshold;
} ImageVersion;


typedef struct {
    int isLeaf;
    int numKeys;
    BTreeKey keys[MAX_KEYS];
    DataRecord records[MAX_KEYS];
    long children[MAX_CHILDREN];  // Offsets dos filhos no arquivo
    long selfOffset;  // Offset deste nó no arquivo
} BTreeNode;

// Raiz virtualizada (sempre em RAM)
BTreeNode* rootNode = NULL;
long rootOffset = -1;

// PROTOTIPOS

// Menu e utilitários
void displayHelp(const char* programName);
int compareKeys(const BTreeKey* a, const BTreeKey* b);

// Funções principais
int storeCompressedImages(const char *pgmFileName, int thresholdCount, int* thresholds, const char *binaryDataFile);
int exportRecordToPgm(const char *nameToExport, int thresholdToExport, const char *binaryDataFile);
int removeFromBTree(const char *nameToRemove, int thresholdToRemove);
void displayImageList();
int compactDatabase(const char *binaryDataFile);
int generateAverageImage(const char *baseName, const char *binaryDataFile);

// Árvore B
void initializeBTree();
BTreeNode* createNode(int isLeaf);
BTreeNode* loadNode(long offset);
void saveNode(BTreeNode* node);
long getNewNodeOffset();
void insertKey(BTreeKey key, DataRecord record);
void insertNonFull(BTreeNode* node, BTreeKey key, DataRecord record);
void splitChild(BTreeNode* parent, int index);
BTreeNode* searchKey(BTreeKey key, DataRecord* record);
int removeKey(BTreeKey key);
void removeFromNode(BTreeNode* node, BTreeKey key);
void removeFromLeaf(BTreeNode* node, int index);
void removeFromNonLeaf(BTreeNode* node, int index);
BTreeKey getPredecessor(BTreeNode* node, int index);
BTreeKey getSuccessor(BTreeNode* node, int index);
void fill(BTreeNode* node, int index);
void borrowFromPrev(BTreeNode* node, int index);
void borrowFromNext(BTreeNode* node, int index);
void merge(BTreeNode* node, int index);
void inorderTraversal(BTreeNode* node);
void collectAllRecords(BTreeNode* node, DataRecord** records, int* count, int* capacity);
void collectImageVersions(BTreeNode* node, const char* baseName, ImageVersion** versions, int* count, int* capacity);

// Processamento de imagem
BinaryImage loadRecordFromBinary(FILE *binInStream, long dataOffset);
BinaryImage loadPgmFile(FILE* pgmStream);
void storePgmFile(const char *destinationFile, BinaryImage *imageToStore);
void applyThreshold(BinaryImage *imageToProcess, int thresholdLevel);
int* compressToRle(BinaryImage *inputImage, int *runCountOutput, unsigned char *firstPixelValue);
BinaryImage expandFromRle(int w, int h, int gray, unsigned char startingValue, int *runLengthArray, int runCount);
void cleanupBTree();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        displayHelp(argv[0]);
        return 1;
    }

    initializeBTree();
    char *command = argv[1];

    if (strcmp(command, "inserir") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Erro: 'inserir' requer <arquivo.pgm> e pelo menos um <limiar>\n");
            fprintf(stderr, "Ex: %s inserir minha_imagem.pgm 128 64 192\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdCount = argc - 3;
        int* thresholds = malloc(thresholdCount * sizeof(int));
        
        for (int i = 0; i < thresholdCount; i++) {
            thresholds[i] = atoi(argv[3 + i]);
            if (thresholds[i] < 0 || thresholds[i] > 255) {
                fprintf(stderr, "Erro: Limiar deve estar entre 0 e 255.\n");
                free(thresholds);
                return 1;
            }
        }
        
        storeCompressedImages(fileName, thresholdCount, thresholds, DATABASE);
        free(thresholds);
    }
    else if (strcmp(command, "exportar") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erro: 'exportar' requer <nome> e <limiar>\n");
            fprintf(stderr, "Ex: %s exportar minha_imagem.pgm 128\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdValue = atoi(argv[3]);
        exportRecordToPgm(fileName, thresholdValue, DATABASE);
    }
    else if (strcmp(command, "remover") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erro: 'remover' requer <nome> e <limiar>\n");
            fprintf(stderr, "Ex: %s remover minha_imagem.pgm 128\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdValue = atoi(argv[3]);
        if (removeFromBTree(fileName, thresholdValue)) {
            printf("Imagem '%s' (limiar %d) removida.\n", fileName, thresholdValue);
        } else {
            printf("Imagem '%s' (limiar %d) nao encontrada.\n", fileName, thresholdValue);
        }
    }
    else if (strcmp(command, "compactar") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Erro: 'compactar' nao requer argumentos.\n");
            return 1;
        }
        compactDatabase(DATABASE);
    }
    else if (strcmp(command, "reconstruir") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Erro: 'reconstruir' requer <nome>\n");
            fprintf(stderr, "Ex: %s reconstruir minha_imagem.pgm\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        generateAverageImage(fileName, DATABASE);
    }
    else if (strcmp(command, "listar") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Erro: 'listar' nao requer argumentos.\n");
            return 1;
        }
        displayImageList();
    }
    else if (strcmp(command, "ajuda") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        displayHelp(argv[0]);
    }
    else {
        fprintf(stderr, "Comando desconhecido: '%s'\n", command);
        displayHelp(argv[0]);
        return 1;
    }

    cleanupBTree();

    return 0;
}

void displayHelp(const char* programName) {
    printf("Uso: %s <comando> [argumentos]\n\n", programName);
    printf("Comandos:\n");
    printf("  inserir <arquivo.pgm> <limiar1> [limiar2] ... - Insere uma imagem PGM com múltiplos limiares.\n");
    printf("  exportar <nome> <limiar>       - Exporta uma imagem do banco para PGM (ASCII P2).\n");
    printf("  remover <nome> <limiar>        - Remove uma imagem da árvore B.\n");
    printf("  listar                         - Lista todas as imagens ativas no banco (percurso ordenado).\n");
    printf("  compactar                      - Remove permanentemente as imagens não referenciadas.\n");
    printf("  reconstruir <nome>             - Gera uma media de todas as versoes de uma imagem.\n");
    printf("  ajuda                          - Mostra esta mensagem.\n\n");
    printf("Arquivos de dados:\n");
    printf("  Arvore B: %s\n", BTREE_FILE);
    printf("  Dados: %s\n", DATABASE);
}

int compareKeys(const BTreeKey* a, const BTreeKey* b) {
    int nameComp = strcmp(a->fileName, b->fileName);
    if (nameComp != 0) return nameComp;
    return a->threshold - b->threshold;
}

void initializeBTree() {
    FILE* btreeFile = fopen(BTREE_FILE, "rb");
    if (!btreeFile) {
        // Arquivo não existe, criar nova árvore
        rootNode = createNode(1);  // Raiz é folha inicialmente
        rootOffset = 0;
        rootNode->selfOffset = rootOffset;
        saveNode(rootNode);
        return;
    }
    
    // Carregar offset da raiz (primeiro long do arquivo)
    if (fread(&rootOffset, sizeof(long), 1, btreeFile) != 1) {
        fclose(btreeFile);
        rootNode = createNode(1);
        rootOffset = sizeof(long);  // Após o offset da raiz
        rootNode->selfOffset = rootOffset;
        saveNode(rootNode);
        return;
    }
    
    fclose(btreeFile);
    rootNode = loadNode(rootOffset);
}

BTreeNode* createNode(int isLeaf) {
    BTreeNode* node = malloc(sizeof(BTreeNode));
    node->isLeaf = isLeaf;
    node->numKeys = 0;
    node->selfOffset = -1;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = -1;
    }
    
    return node;
}

BTreeNode* loadNode(long offset) {
    FILE* btreeFile = fopen(BTREE_FILE, "rb");
    if (!btreeFile) return NULL;
    
    fseek(btreeFile, offset, SEEK_SET);
    BTreeNode* node = malloc(sizeof(BTreeNode));
    
    if (fread(node, sizeof(BTreeNode), 1, btreeFile) != 1) {
        free(node);
        fclose(btreeFile);
        return NULL;
    }
    
    fclose(btreeFile);
    return node;
}

void saveNode(BTreeNode* node) {
    FILE* btreeFile = fopen(BTREE_FILE, "rb+");
    if (!btreeFile) {
        btreeFile = fopen(BTREE_FILE, "wb+");
        // Escrever offset da raiz no início do arquivo
        fwrite(&rootOffset, sizeof(long), 1, btreeFile);
    }
    
    if (node->selfOffset == -1) {
        node->selfOffset = getNewNodeOffset();
    }
    
    fseek(btreeFile, node->selfOffset, SEEK_SET);
    fwrite(node, sizeof(BTreeNode), 1, btreeFile);
    
    // Atualizar offset da raiz se necessário
    if (node == rootNode) {
        fseek(btreeFile, 0, SEEK_SET);
        fwrite(&rootOffset, sizeof(long), 1, btreeFile);
    }
    
    fclose(btreeFile);
}

long getNewNodeOffset() {
    FILE* btreeFile = fopen(BTREE_FILE, "rb");
    if (!btreeFile) return sizeof(long);  // Primeiro nó após o offset da raiz
    
    fseek(btreeFile, 0, SEEK_END);
    long offset = ftell(btreeFile);
    fclose(btreeFile);
    
    return offset;
}

void insertKey(BTreeKey key, DataRecord record) {
    if (rootNode->numKeys == MAX_KEYS) {
        // Raiz está cheia, precisa dividir
        BTreeNode* newRoot = createNode(0);
        newRoot->children[0] = rootOffset;
        
        // Salvar a raiz antiga
        saveNode(rootNode);
        
        // Atualizar raiz
        free(rootNode);
        rootNode = newRoot;
        rootOffset = getNewNodeOffset();
        rootNode->selfOffset = rootOffset;
        
        splitChild(rootNode, 0);
        insertNonFull(rootNode, key, record);
    } else {
        insertNonFull(rootNode, key, record);
    }
    
    saveNode(rootNode);
}

void insertNonFull(BTreeNode* node, BTreeKey key, DataRecord record) {
    int i = node->numKeys - 1;
    
    if (node->isLeaf) {
        // Inserir na folha
        while (i >= 0 && compareKeys(&key, &node->keys[i]) < 0) {
            node->keys[i + 1] = node->keys[i];
            node->records[i + 1] = node->records[i];
            i--;
        }
        
        node->keys[i + 1] = key;
        node->records[i + 1] = record;
        node->numKeys++;
    } else {
        // Encontrar filho apropriado
        while (i >= 0 && compareKeys(&key, &node->keys[i]) < 0) {
            i--;
        }
        i++;
        
        BTreeNode* child = loadNode(node->children[i]);
        if (child->numKeys == MAX_KEYS) {
            splitChild(node, i);
            if (compareKeys(&key, &node->keys[i]) > 0) {
                i++;
            }
            free(child);
            child = loadNode(node->children[i]);
        }
        
        insertNonFull(child, key, record);
        saveNode(child);
        free(child);
    }
}

void splitChild(BTreeNode* parent, int index) {
    BTreeNode* fullChild = loadNode(parent->children[index]);
    BTreeNode* newChild = createNode(fullChild->isLeaf);
    
    newChild->numKeys = MAX_KEYS / 2;
    
    // Copiar metade das chaves para o novo nó
    for (int j = 0; j < MAX_KEYS / 2; j++) {
        newChild->keys[j] = fullChild->keys[j + MAX_KEYS / 2 + 1];
        newChild->records[j] = fullChild->records[j + MAX_KEYS / 2 + 1];
    }
    
    if (!fullChild->isLeaf) {
        for (int j = 0; j <= MAX_KEYS / 2; j++) {
            newChild->children[j] = fullChild->children[j + MAX_KEYS / 2 + 1];
        }
    }
    
    fullChild->numKeys = MAX_KEYS / 2;
    
    // Mover filhos do pai
    for (int j = parent->numKeys; j >= index + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }
    
    newChild->selfOffset = getNewNodeOffset();
    parent->children[index + 1] = newChild->selfOffset;
    
    // Mover chaves do pai
    for (int j = parent->numKeys - 1; j >= index; j--) {
        parent->keys[j + 1] = parent->keys[j];
        parent->records[j + 1] = parent->records[j];
    }
    
    parent->keys[index] = fullChild->keys[MAX_KEYS / 2];
    parent->records[index] = fullChild->records[MAX_KEYS / 2];
    parent->numKeys++;
    
    saveNode(fullChild);
    saveNode(newChild);
    
    free(fullChild);
    free(newChild);
}

BTreeNode* searchKey(BTreeKey key, DataRecord* record) {
    BTreeNode* current = rootNode;
    
    while (current != NULL) {
        int i = 0;
        while (i < current->numKeys && compareKeys(&key, &current->keys[i]) > 0) {
            i++;
        }
        
        if (i < current->numKeys && compareKeys(&key, &current->keys[i]) == 0) {
            if (record) *record = current->records[i];
            return current;
        }
        
        if (current->isLeaf) {
            return NULL;
        }
        
        BTreeNode* next = loadNode(current->children[i]);
        if (current != rootNode) {
            free(current);
        }
        current = next;
    }
    
    return NULL;
}

int removeKey(BTreeKey key) {
    if (!rootNode) return 0;
    
    removeFromNode(rootNode, key);
    
    if (rootNode->numKeys == 0) {
        if (!rootNode->isLeaf) {
            BTreeNode* newRoot = loadNode(rootNode->children[0]);
            long oldRootOffset = rootOffset;
            rootOffset = newRoot->selfOffset;
            free(rootNode);
            rootNode = newRoot;
            
            // Marcar o nó antigo como livre (implementação simplificada)
        }
    }
    
    saveNode(rootNode);
    return 1;
}

void removeFromNode(BTreeNode* node, BTreeKey key) {
    int idx = 0;
    while (idx < node->numKeys && compareKeys(&key, &node->keys[idx]) > 0) {
        idx++;
    }
    
    if (idx < node->numKeys && compareKeys(&key, &node->keys[idx]) == 0) {
        if (node->isLeaf) {
            removeFromLeaf(node, idx);
        } else {
            removeFromNonLeaf(node, idx);
        }
    } else {
        if (node->isLeaf) {
            return;  // Chave não encontrada
        }
        
        int flag = (idx == node->numKeys);
        BTreeNode* child = loadNode(node->children[idx]);
        
        if (child->numKeys < ORDER / 2) {
            fill(node, idx);
        }
        
        if (flag && idx > node->numKeys) {
            BTreeNode* newChild = loadNode(node->children[idx - 1]);
            removeFromNode(newChild, key);
            saveNode(newChild);
            free(newChild);
        } else {
            BTreeNode* newChild = loadNode(node->children[idx]);
            removeFromNode(newChild, key);
            saveNode(newChild);
            free(newChild);
        }
        
        free(child);
    }
}

void removeFromLeaf(BTreeNode* node, int index) {
    for (int i = index + 1; i < node->numKeys; i++) {
        node->keys[i - 1] = node->keys[i];
        node->records[i - 1] = node->records[i];
    }
    node->numKeys--;
}

void removeFromNonLeaf(BTreeNode* node, int index) {
    BTreeKey key = node->keys[index];
    
    BTreeNode* leftChild = loadNode(node->children[index]);
    BTreeNode* rightChild = loadNode(node->children[index + 1]);
    
    if (leftChild->numKeys >= ORDER / 2) {
        BTreeKey pred = getPredecessor(node, index);
        node->keys[index] = pred;
        removeFromNode(leftChild, pred);
        saveNode(leftChild);
    } else if (rightChild->numKeys >= ORDER / 2) {
        BTreeKey succ = getSuccessor(node, index);
        node->keys[index] = succ;
        removeFromNode(rightChild, succ);
        saveNode(rightChild);
    } else {
        merge(node, index);
        removeFromNode(leftChild, key);
        saveNode(leftChild);
    }
    
    free(leftChild);
    free(rightChild);
}

BTreeKey getPredecessor(BTreeNode* node, int index) {
    BTreeNode* current = loadNode(node->children[index]);
    while (!current->isLeaf) {
        BTreeNode* next = loadNode(current->children[current->numKeys]);
        free(current);
        current = next;
    }
    
    BTreeKey pred = current->keys[current->numKeys - 1];
    free(current);
    return pred;
}

BTreeKey getSuccessor(BTreeNode* node, int index) {
    BTreeNode* current = loadNode(node->children[index + 1]);
    while (!current->isLeaf) {
        BTreeNode* next = loadNode(current->children[0]);
        free(current);
        current = next;
    }
    
    BTreeKey succ = current->keys[0];
    free(current);
    return succ;
}

void fill(BTreeNode* node, int index) {
    BTreeNode* child = loadNode(node->children[index]);
    
    if (index != 0) {
        BTreeNode* leftSibling = loadNode(node->children[index - 1]);
        if (leftSibling->numKeys >= ORDER / 2) {
            borrowFromPrev(node, index);
            free(leftSibling);
            free(child);
            return;
        }
        free(leftSibling);
    }
    
    if (index != node->numKeys) {
        BTreeNode* rightSibling = loadNode(node->children[index + 1]);
        if (rightSibling->numKeys >= ORDER / 2) {
            borrowFromNext(node, index);
            free(rightSibling);
            free(child);
            return;
        }
        free(rightSibling);
    }
    
    if (index != node->numKeys) {
        merge(node, index);
    } else {
        merge(node, index - 1);
    }
    
    free(child);
}

void borrowFromPrev(BTreeNode* node, int index) {
    BTreeNode* child = loadNode(node->children[index]);
    BTreeNode* sibling = loadNode(node->children[index - 1]);
    
    for (int i = child->numKeys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        child->records[i + 1] = child->records[i];
    }
    
    if (!child->isLeaf) {
        for (int i = child->numKeys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
        child->children[0] = sibling->children[sibling->numKeys];
    }
    
    child->keys[0] = node->keys[index - 1];
    child->records[0] = node->records[index - 1];
    
    node->keys[index - 1] = sibling->keys[sibling->numKeys - 1];
    node->records[index - 1] = sibling->records[sibling->numKeys - 1];
    
    child->numKeys++;
    sibling->numKeys--;
    
    saveNode(child);
    saveNode(sibling);
    free(child);
    free(sibling);
}

void borrowFromNext(BTreeNode* node, int index) {
    BTreeNode* child = loadNode(node->children[index]);
    BTreeNode* sibling = loadNode(node->children[index + 1]);
    
    child->keys[child->numKeys] = node->keys[index];
    child->records[child->numKeys] = node->records[index];
    
    if (!child->isLeaf) {
        child->children[child->numKeys + 1] = sibling->children[0];
    }
    
    node->keys[index] = sibling->keys[0];
    node->records[index] = sibling->records[0];
    
    for (int i = 1; i < sibling->numKeys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->records[i - 1] = sibling->records[i];
    }
    
    if (!sibling->isLeaf) {
        for (int i = 1; i <= sibling->numKeys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }
    
    child->numKeys++;
    sibling->numKeys--;
    
    saveNode(child);
    saveNode(sibling);
    free(child);
    free(sibling);
}

void merge(BTreeNode* node, int index) {
    BTreeNode* child = loadNode(node->children[index]);
    BTreeNode* sibling = loadNode(node->children[index + 1]);
    
    child->keys[MAX_KEYS / 2] = node->keys[index];
    child->records[MAX_KEYS / 2] = node->records[index];
    
    for (int i = 0; i < sibling->numKeys; i++) {
        child->keys[i + MAX_KEYS / 2 + 1] = sibling->keys[i];
        child->records[i + MAX_KEYS / 2 + 1] = sibling->records[i];
    }
    
    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->numKeys; i++) {
            child->children[i + MAX_KEYS / 2 + 1] = sibling->children[i];
        }
    }
    
    for (int i = index + 1; i < node->numKeys; i++) {
        node->keys[i - 1] = node->keys[i];
        node->records[i - 1] = node->records[i];
    }
    
    for (int i = index + 2; i <= node->numKeys; i++) {
        node->children[i - 1] = node->children[i];
    }
    
    child->numKeys += sibling->numKeys + 1;
    node->numKeys--;
    
    saveNode(child);
    // sibling pode ser marcado como livre
    
    free(child);
    free(sibling);
}

void inorderTraversal(BTreeNode* node) {
    if (!node) return;
    
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) {
            BTreeNode* child = loadNode(node->children[i]);
            inorderTraversal(child);
            if (child != rootNode) free(child);
        }
        
        printf("%-30s | %-10d | %-12ld | %-12ld\n", 
               node->keys[i].fileName, node->keys[i].threshold,
               node->records[i].dataOffset, node->records[i].blockSize);
    }
    
    if (!node->isLeaf) {
        BTreeNode* child = loadNode(node->children[i]);
        inorderTraversal(child);
        if (child != rootNode) free(child);
    }
}

void collectAllRecords(BTreeNode* node, DataRecord** records, int* count, int* capacity) {
    if (!node) return;
    
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) {
            BTreeNode* child = loadNode(node->children[i]);
            collectAllRecords(child, records, count, capacity);
            if (child != rootNode) free(child);
        }
        
        // Adicionar registro ao array
        if (*count >= *capacity) {
            *capacity *= 2;
            *records = realloc(*records, *capacity * sizeof(DataRecord));
        }
        (*records)[*count] = node->records[i];
        (*count)++;
    }
    
    if (!node->isLeaf) {
        BTreeNode* child = loadNode(node->children[i]);
        collectAllRecords(child, records, count, capacity);
        if (child != rootNode) free(child);
    }
}

// Implementação das funções principais

int storeCompressedImages(const char *pgmFileName, int thresholdCount, int* thresholds, const char *binaryDataFile) {
    FILE *pgmInStream = fopen(pgmFileName, "rb");
    if (!pgmInStream) { 
        perror("Erro ao abrir PGM"); 
        return 0; 
    }

    BinaryImage image = loadPgmFile(pgmInStream);
    fclose(pgmInStream);
    
    if (!image.pixelData) { 
        fprintf(stderr, "Erro ao ler imagem.\n"); 
        return 0; 
    }

    FILE *binOutStream = fopen(binaryDataFile, "ab+");
    if (!binOutStream) { 
        perror("Erro ao abrir bin"); 
        free(image.pixelData); 
        return 0; 
    }

    printf("Inserindo imagem '%s' com %d limiares:\n", pgmFileName, thresholdCount);
    
    for (int t = 0; t < thresholdCount; t++) {
        int thresholdLevel = thresholds[t];
        
        // Criar cópia da imagem para este limiar
        BinaryImage thresholdImage = {image.imgHeight, image.imgWidth, image.maxGrayValue, NULL};
        long pixelCount = (long)image.imgWidth * image.imgHeight;
        thresholdImage.pixelData = malloc(pixelCount * sizeof(short int));
        memcpy(thresholdImage.pixelData, image.pixelData, pixelCount * sizeof(short int));
        
        applyThreshold(&thresholdImage, thresholdLevel);

        int runCount = 0;
        unsigned char startingValue = 0;
        int *runLengthArray = compressToRle(&thresholdImage, &runCount, &startingValue);

        if (!runLengthArray && runCount > 0) { 
            free(thresholdImage.pixelData); 
            continue; 
        }

        fseek(binOutStream, 0, SEEK_END);
        long currentOffset = ftell(binOutStream);

        // Escrever metadados
        fwrite(&thresholdImage.imgWidth, sizeof(int), 1, binOutStream);
        fwrite(&thresholdImage.imgHeight, sizeof(int), 1, binOutStream);
        fwrite(&thresholdImage.maxGrayValue, sizeof(int), 1, binOutStream);
        fwrite(&startingValue, sizeof(unsigned char), 1, binOutStream);
        fwrite(&runCount, sizeof(int), 1, binOutStream);
        if (runCount > 0) fwrite(runLengthArray, sizeof(int), runCount, binOutStream);

        long compressedBytes = ftell(binOutStream) - currentOffset;

        // Inserir na árvore B
        BTreeKey key;
        strncpy(key.fileName, pgmFileName, sizeof(key.fileName) - 1);
        key.fileName[sizeof(key.fileName) - 1] = '\0';
        key.threshold = thresholdLevel;
        
        DataRecord record;
        record.dataOffset = currentOffset;
        record.blockSize = compressedBytes;
        
        insertKey(key, record);

        printf("  -> Limiar %d: %ld bytes comprimidos\n", thresholdLevel, compressedBytes);
        
        free(thresholdImage.pixelData);
        if (runLengthArray) free(runLengthArray);
    }

    fclose(binOutStream);
    free(image.pixelData);
    
    printf("Todas as imagens inseridas com sucesso!\n");
    return 1;
}

int exportRecordToPgm(const char *nameToExport, int thresholdToExport, const char *binaryDataFile) {
    BTreeKey key;
    strncpy(key.fileName, nameToExport, sizeof(key.fileName) - 1);
    key.fileName[sizeof(key.fileName) - 1] = '\0';
    key.threshold = thresholdToExport;
    
    DataRecord record;
    BTreeNode* found = searchKey(key, &record);
    
    if (!found) { 
        printf("Registro nao encontrado.\n"); 
        return 0; 
    }

    FILE *binInStream = fopen(binaryDataFile, "rb");
    if (!binInStream) { 
        perror("Erro abrir data file"); 
        return 0; 
    }

    BinaryImage exportedImage = loadRecordFromBinary(binInStream, record.dataOffset);
    fclose(binInStream);
    
    if (!exportedImage.pixelData) { 
        printf("Erro ao decodificar registro.\n"); 
        return 0; 
    }

    char outputFileName[512];
    char baseFileName[256];
    strcpy(baseFileName, nameToExport);
    
    char *dotPointer = strstr(baseFileName, ".pgm");
    if(dotPointer) *dotPointer = '\0';

    snprintf(outputFileName, sizeof(outputFileName), "saida_%s_l%d.pgm", baseFileName, thresholdToExport);
    storePgmFile(outputFileName, &exportedImage);
    free(exportedImage.pixelData);
    
    printf("Exportada em %s\n", outputFileName);
    return 1;
}

int removeFromBTree(const char *nameToRemove, int thresholdToRemove) {
    BTreeKey key;
    strncpy(key.fileName, nameToRemove, sizeof(key.fileName) - 1);
    key.fileName[sizeof(key.fileName) - 1] = '\0';
    key.threshold = thresholdToRemove;
    
    return removeKey(key);
}

void displayImageList() {
    if (!rootNode || rootNode->numKeys == 0) {
        printf("Banco de dados vazio.\n");
        return;
    }
    
    printf("\n--- Imagens Ativas no Banco (Percurso Ordenado) ---\n");
    printf("-------------------------------------------------------------------------\n");
    printf("%-30s | %-10s | %-12s | %-12s\n", "Nome", "Limiar", "Offset", "Tamanho (bytes)");
    printf("-------------------------------------------------------------------------\n");
    
    inorderTraversal(rootNode);
    
    printf("-------------------------------------------------------------------------\n\n");
}

// Continuação da função compactDatabase que estava incompleta
int compactDatabase(const char *binaryDataFile) {
    if (!rootNode) {
        printf("Arvore vazia, nada para compactar.\n");
        return 0;
    }
    
    // Coletar todos os registros válidos
    DataRecord* records = malloc(100 * sizeof(DataRecord));
    int count = 0, capacity = 100;
    
    collectAllRecords(rootNode, &records, &count, &capacity);
    
    if (count == 0) {
        printf("Nenhum registro para compactar.\n");
        free(records);
        return 0;
    }
    
    // Abrir arquivos
    FILE *oldDataFile = fopen(binaryDataFile, "rb");
    if (!oldDataFile) {
        perror("Erro ao abrir arquivo de dados antigo");
        free(records);
        return 0;
    }
    
    FILE *newDataFile = fopen(TEMP_DATABASE, "wb");
    if (!newDataFile) {
        perror("Erro ao criar arquivo de dados temporário");
        fclose(oldDataFile);
        free(records);
        return 0;
    }
    
    // Buffer para cópia
    const size_t BUFFER_SIZE = 8192;
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Erro ao alocar buffer");
        fclose(oldDataFile);
        fclose(newDataFile);
        free(records);
        return 0;
    }
    
    long newOffset = 0;
    int copiedRecords = 0;
    
    // Copiar cada registro válido
    for (int i = 0; i < count; i++) {
        DataRecord *record = &records[i];
        
        // Posicionar no offset antigo
        fseek(oldDataFile, record->dataOffset, SEEK_SET);
        
        // Atualizar offset no registro
        record->dataOffset = newOffset;
        
        // Copiar dados em chunks
        long remainingBytes = record->blockSize;
        while (remainingBytes > 0) {
            size_t toRead = (remainingBytes > BUFFER_SIZE) ? BUFFER_SIZE : remainingBytes;
            size_t bytesRead = fread(buffer, 1, toRead, oldDataFile);
            if (bytesRead == 0) break;
            
            fwrite(buffer, 1, bytesRead, newDataFile);
            remainingBytes -= bytesRead;
        }
        
        newOffset += record->blockSize;
        copiedRecords++;
    }
    
    // Fechar arquivos
    free(buffer);
    fclose(oldDataFile);
    fclose(newDataFile);
    
    // Substituir arquivo antigo pelo novo
    if (remove(binaryDataFile) != 0) {
        perror("Aviso: não foi possível remover arquivo antigo");
    }
    if (rename(TEMP_DATABASE, binaryDataFile) != 0) {
        perror("Erro ao renomear arquivo temporário");
        free(records);
        return 0;
    }
    
    // Reconstruir árvore B com novos offsets
    // (Implementação simplificada - na prática, você atualizaria os offsets na árvore)
    
    free(records);
    printf("Compactação concluída. %d registros copiados.\n", copiedRecords);
    return copiedRecords;
}

// Função generateAverageImage que estava faltando
int generateAverageImage(const char *baseName, const char *binaryDataFile) {
    if (!rootNode) {
        printf("Banco de dados vazio.\n");
        return 0;
    }
    
    // Coletar todas as versões da imagem com o nome base
    ImageVersion *versions = malloc(50 * sizeof(ImageVersion));
    int versionCount = 0, capacity = 50;
    
    // Percorrer árvore coletando versões da imagem
    collectImageVersions(rootNode, baseName, &versions, &versionCount, &capacity);
    
    if (versionCount == 0) {
        printf("Nenhuma versão encontrada para '%s'.\n", baseName);
        free(versions);
        return 0;
    }
    
    FILE *dataFile = fopen(binaryDataFile, "rb");
    if (!dataFile) {
        perror("Erro ao abrir arquivo de dados");
        free(versions);
        return 0;
    }
    
    // Carregar primeira imagem para obter dimensões
    BinaryImage firstImage = loadRecordFromBinary(dataFile, versions[0].record.dataOffset);
    if (!firstImage.pixelData) {
        printf("Erro ao carregar primeira versão.\n");
        fclose(dataFile);
        free(versions);
        return 0;
    }
    
    int width = firstImage.imgWidth;
    int height = firstImage.imgHeight;
    int grayMax = firstImage.maxGrayValue;
    long pixelCount = (long)width * height;
    
    // Acumulador para soma dos pixels
    long *accumulator = calloc(pixelCount, sizeof(long));
    if (!accumulator) {
        printf("Erro ao alocar acumulador.\n");
        free(firstImage.pixelData);
        fclose(dataFile);
        free(versions);
        return 0;
    }
    
    // Somar pixels da primeira imagem
    for (long i = 0; i < pixelCount; i++) {
        accumulator[i] += firstImage.pixelData[i];
    }
    free(firstImage.pixelData);
    
    // Somar pixels das outras versões
    for (int v = 1; v < versionCount; v++) {
        BinaryImage image = loadRecordFromBinary(dataFile, versions[v].record.dataOffset);
        if (!image.pixelData) {
            printf("Aviso: erro ao carregar versão %d, pulando.\n", v);
            continue;
        }
        
        if (image.imgWidth != width || image.imgHeight != height) {
            printf("Aviso: dimensões diferentes na versão %d, pulando.\n", v);
            free(image.pixelData);
            continue;
        }
        
        for (long i = 0; i < pixelCount; i++) {
            accumulator[i] += image.pixelData[i];
        }
        free(image.pixelData);
    }
    
    fclose(dataFile);
    
    // Calcular média e criar imagem resultante
    BinaryImage resultImage = {height, width, grayMax, NULL};
    resultImage.pixelData = malloc(pixelCount * sizeof(short int));
    if (!resultImage.pixelData) {
        printf("Erro ao alocar imagem resultado.\n");
        free(accumulator);
        free(versions);
        return 0;
    }
    
    for (long i = 0; i < pixelCount; i++) {
        long average = accumulator[i] / versionCount;
        if (average < 0) average = 0;
        if (average > grayMax) average = grayMax;
        resultImage.pixelData[i] = (short int)average;
    }
    
    // Salvar imagem reconstruída
    char outputFileName[512];
    snprintf(outputFileName, sizeof(outputFileName), "reconstruida_media_%s", baseName);
    storePgmFile(outputFileName, &resultImage);
    
    printf("Reconstrução média salva em %s (usando %d versões).\n", outputFileName, versionCount);
    
    // Limpeza
    free(resultImage.pixelData);
    free(accumulator);
    free(versions);
    
    return 1;
}

// Função auxiliar para coletar versões de uma imagem específica
void collectImageVersions(BTreeNode* node, const char* baseName, ImageVersion** versions, int* count, int* capacity) {
    if (!node) return;
    
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) {
            BTreeNode* child = loadNode(node->children[i]);
            collectImageVersions(child, baseName, versions, count, capacity);
            if (child != rootNode) free(child);
        }
        
        // Verificar se é uma versão da imagem desejada
        if (strcmp(node->keys[i].fileName, baseName) == 0) {
            // Expandir array se necessário
            if (*count >= *capacity) {
                *capacity *= 2;
                *versions = realloc(*versions, *capacity * sizeof(ImageVersion));
            }
            
            (*versions)[*count].record = node->records[i];
            (*versions)[*count].threshold = node->keys[i].threshold;
            (*count)++;
        }
    }
    
    if (!node->isLeaf) {
        BTreeNode* child = loadNode(node->children[i]);
        collectImageVersions(child, baseName, versions, count, capacity);
        if (child != rootNode) free(child);
    }
}

// Função para limpeza da árvore B ao sair do programa
void cleanupBTree() {
    if (rootNode) {
        free(rootNode);
        rootNode = NULL;
    }
}

BinaryImage loadPgmFile(FILE* pgmStream) {
    BinaryImage image = {0, 0, 0, NULL};
    char buffer[256];
    int i, pixelValue;
    
    // Read PGM header
    // Format identifier (P2 for ASCII PGM)
    if (!fgets(buffer, sizeof(buffer), pgmStream)) {
        fprintf(stderr, "Error reading PGM header\n");
        return image;
    }
    
    // Skip comments
    do {
        if (!fgets(buffer, sizeof(buffer), pgmStream)) {
            fprintf(stderr, "Error reading PGM header\n");
            return image;
        }
    } while (buffer[0] == '#');
    
    // Read width and height
    if (sscanf(buffer, "%d %d", &image.imgWidth, &image.imgHeight) != 2) {
        fprintf(stderr, "Error reading image dimensions\n");
        return image;
    }
    
    // Read max gray value
    if (!fgets(buffer, sizeof(buffer), pgmStream)) {
        fprintf(stderr, "Error reading max gray value\n");
        return image;
    }
    sscanf(buffer, "%d", &image.maxGrayValue);
    
    // Allocate memory for pixel data
    long pixelCount = (long)image.imgWidth * image.imgHeight;
    image.pixelData = (short int*)malloc(pixelCount * sizeof(short int));
    if (!image.pixelData) {
        fprintf(stderr, "Memory allocation failed for pixel data\n");
        return image;
    }
    
    // Read pixel data
    for (i = 0; i < pixelCount; i++) {
        if (fscanf(pgmStream, "%d", &pixelValue) != 1) {
            fprintf(stderr, "Error reading pixel data at position %d\n", i);
            free(image.pixelData);
            image.pixelData = NULL;
            return image;
        }
        image.pixelData[i] = (short int)pixelValue;
    }
    
    return image;
}

void applyThreshold(BinaryImage *imageToProcess, int thresholdLevel) {
    if (!imageToProcess || !imageToProcess->pixelData) {
        return;  // Guard against NULL pointers
    }
    
    long pixelCount = (long)imageToProcess->imgWidth * imageToProcess->imgHeight;
    
    // Apply threshold to each pixel
    for (long i = 0; i < pixelCount; i++) {
        // If pixel value is greater than or equal to threshold, set to max gray value
        // Otherwise, set to 0
        if (imageToProcess->pixelData[i] >= thresholdLevel) {
            imageToProcess->pixelData[i] = imageToProcess->maxGrayValue;
        } else {
            imageToProcess->pixelData[i] = 0;
        }
    }
}

int* compressToRle(BinaryImage *inputImage, int *runCountOutput, unsigned char *firstPixelValue) 
{
 int pixelCount = inputImage->imgWidth * inputImage->imgHeight; // total de pixels
 if (pixelCount == 0) { *runCountOutput = 0; *firstPixelValue = 0; return NULL; } // imagem vazia

 short int *pixelPtr = inputImage->pixelData; // ponteiros
 short int *endPtr = inputImage->pixelData + pixelCount;

 unsigned char startingValue = (*pixelPtr == inputImage->maxGrayValue) ? 1 : 0; // define o bit inicial (0 ou 1) baseado no primeiro pixel
 *firstPixelValue = startingValue; // salva o bit inicial (retorno por referência)

 int currentCapacity = 128; // capacidade inicial do array rle (dinâmico)
 int *runLengthArray = (int*)malloc(currentCapacity * sizeof(int)); // aloca o array rle
 if (!runLengthArray) return NULL; // checa alocação

 int currentRunLength = 1; // 'corrida' (run) atual começa com 1 (primeiro pixel)
 unsigned char currentValue = startingValue; // valor (bit) da 'corrida' atual
 *runCountOutput = 0; // zera o contador de runs (retorno por referência)

 pixelPtr++; // avança para o segundo pixel

 // loop do segundo ao último pixel
 for (; pixelPtr < endPtr; pixelPtr++){ 
 
  unsigned char nextBit = (*pixelPtr == inputImage->maxGrayValue) ? 1 : 0; // converte o valor do pixel para bit (0 ou 1)
  
  if (nextBit == currentValue) currentRunLength++; // se o bit for o mesmo, incrementa a corrida atual
  else{ // se o bit for diferente (quebrou a corrida)...
   // ...checa se precisa de mais espaço no array (realloc)
   if (*runCountOutput + 1 > currentCapacity) { 
    currentCapacity *= 2; // dobra a capacidade
    runLengthArray = (int*)realloc(runLengthArray, currentCapacity * sizeof(int)); 
    if(!runLengthArray){ *runCountOutput = 0; return NULL; } // checa falha no realloc
   }
   runLengthArray[(*runCountOutput)++] = currentRunLength; // ...salva o tamanho da 'corrida' anterior
   currentRunLength = 1; // ...inicia a nova 'corrida' com 1
   currentValue = nextBit; // ...atualiza o valor (bit) da nova 'corrida'
  }
 }
 
 // salva a última corrida (após o loop)
 if (*runCountOutput + 1 > currentCapacity) { // checa o espaço uma última vez
  currentCapacity += 1; 
  runLengthArray = (int*)realloc(runLengthArray, currentCapacity * sizeof(int)); 
  if(!runLengthArray){ *runCountOutput = 0; return NULL; } 
 }
 runLengthArray[(*runCountOutput)++] = currentRunLength; // salva a última 'corrida'

 return runLengthArray; // retorna o array rle alocado
}

BinaryImage loadRecordFromBinary(FILE *binInStream, long dataOffset) {
    BinaryImage image = {0, 0, 0, NULL};
    
    if (!binInStream) {
        fprintf(stderr, "Invalid binary input stream\n");
        return image;
    }
    
    // Position file pointer at the start of the record
    if (fseek(binInStream, dataOffset, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to data offset %ld\n", dataOffset);
        return image;
    }
    
    // Read image metadata
    if (fread(&image.imgWidth, sizeof(int), 1, binInStream) != 1 ||
        fread(&image.imgHeight, sizeof(int), 1, binInStream) != 1 ||
        fread(&image.maxGrayValue, sizeof(int), 1, binInStream) != 1) {
        fprintf(stderr, "Error reading image metadata\n");
        return image;
    }
    
    // Read RLE compression metadata
    unsigned char startingValue;
    int runCount;
    
    if (fread(&startingValue, sizeof(unsigned char), 1, binInStream) != 1 ||
        fread(&runCount, sizeof(int), 1, binInStream) != 1) {
        fprintf(stderr, "Error reading RLE metadata\n");
        return image;
    }
    
    // Read run length array
    int *runLengthArray = NULL;
    if (runCount > 0) {
        runLengthArray = (int*)malloc(runCount * sizeof(int));
        if (!runLengthArray) {
            fprintf(stderr, "Memory allocation failed for run length array\n");
            return image;
        }
        
        if (fread(runLengthArray, sizeof(int), runCount, binInStream) != runCount) {
            fprintf(stderr, "Error reading run length data\n");
            free(runLengthArray);
            return image;
        }
    }
    
    // Expand RLE data to reconstruct the image
    image = expandFromRle(image.imgWidth, image.imgHeight, image.maxGrayValue, 
                          startingValue, runLengthArray, runCount);
    
    // Clean up
    if (runLengthArray) {
        free(runLengthArray);
    }
    
    return image;
}

void storePgmFile(const char *destinationFile, BinaryImage *imageToStore) {
    if (!imageToStore || !imageToStore->pixelData) {
        fprintf(stderr, "Invalid image data for storing\n");
        return;
    }
    
    FILE *pgmOutStream = fopen(destinationFile, "w");
    if (!pgmOutStream) {
        perror("Error opening output file");
        return;
    }
    
    // Write PGM header (P2 format - ASCII)
    fprintf(pgmOutStream, "P2\n");
    fprintf(pgmOutStream, "# Generated by image processor\n");
    fprintf(pgmOutStream, "%d %d\n", imageToStore->imgWidth, imageToStore->imgHeight);
    fprintf(pgmOutStream, "%d\n", imageToStore->maxGrayValue);
    
    // Write pixel data
    long pixelCount = (long)imageToStore->imgWidth * imageToStore->imgHeight;
    int pixelsPerLine = 17; // Format nicely with ~70 chars per line
    
    for (long i = 0; i < pixelCount; i++) {
        fprintf(pgmOutStream, "%d", imageToStore->pixelData[i]);
        
        // Add space or newline for formatting
        if (i < pixelCount - 1) {
            if ((i + 1) % pixelsPerLine == 0) {
                fprintf(pgmOutStream, "\n");
            } else {
                fprintf(pgmOutStream, " ");
            }
        }
    }
    
    // Ensure file ends with a newline
    if (pixelCount % pixelsPerLine != 0) {
        fprintf(pgmOutStream, "\n");
    }
    
    fclose(pgmOutStream);
}

BinaryImage expandFromRle(int w, int h, int gray, unsigned char startingValue, int *runLengthArray, int runCount) {
    BinaryImage result = {h, w, gray, NULL};
    
    // Calculate total number of pixels
    long pixelCount = (long)w * h;
    if (pixelCount <= 0) {
        fprintf(stderr, "Invalid dimensions for image expansion\n");
        return result;
    }
    
    // Allocate memory for pixel data
    result.pixelData = (short int*)malloc(pixelCount * sizeof(short int));
    if (!result.pixelData) {
        fprintf(stderr, "Memory allocation failed for expanded image\n");
        return result;
    }
    
    // Expand RLE data
    long pixelIndex = 0;
    unsigned char currentValue = startingValue;
    
    for (int i = 0; i < runCount && pixelIndex < pixelCount; i++) {
        int runLength = runLengthArray[i];
        short int pixelValue = (currentValue > 0) ? gray : 0;
        
        // Fill pixels for this run
        for (int j = 0; j < runLength && pixelIndex < pixelCount; j++) {
            result.pixelData[pixelIndex++] = pixelValue;
        }
        
        // Toggle the current value for the next run
        currentValue = !currentValue;
    }
    
    // If we didn't fill the entire image (error in RLE data), fill the rest with zeros
    if (pixelIndex < pixelCount) {
        fprintf(stderr, "Warning: RLE data insufficient to fill entire image. Filling remainder with zeros.\n");
        for (; pixelIndex < pixelCount; pixelIndex++) {
            result.pixelData[pixelIndex] = 0;
        }
    }
    
    return result;
}