#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>
#include <stdbool.h>

// === configurações da b-tree ===

// ordem (t) da b-tree - define quantas chaves cada nó pode ter
// chaves mínimas por nó = t - 1
// chaves máximas por nó = 2*t - 1  
// filhos máximos por nó = 2*t
#define BTREE_ORDER 3 

// tamanho máximo do nome do arquivo (chave)
#define MAX_KEY_SIZE 64 

// valor especial para representar "nulo" em offsets de arquivo
#define INVALID_OFFSET -1L

// === estruturas de dados ===

// registro que guarda informações sobre cada imagem
typedef struct {
    char recordName[50];     // nome do arquivo da imagem
    int thresholdValue;      // limiar usado na binarização
    long dataOffset;         // onde a imagem comprimida está no arquivo .bin
    long blockSize;          // tamanho da imagem comprimida
    int isDeleted;           // flag para deleção preguiçosa (0=válido, 1=deletado)
} IndexRecord;

// nó da b-tree (pode ser folha ou interno)
typedef struct {
    int numKeys;                              // quantas chaves tem neste nó
    int isLeaf;                               // é folha (1) ou nó interno (0)?
    long children[2 * BTREE_ORDER];           // "ponteiros" (offsets) para nós filhos
    IndexRecord records[2 * BTREE_ORDER - 1]; // array com os registros de dados
} BTreeNode;

// cabeçalho do arquivo da b-tree
typedef struct {
    long rootNodeOffset;  // onde está localizado o nó raiz no arquivo
    long nextFreeOffset;  // onde podemos escrever um novo nó (fim do arquivo)
} BTreeHeader;

// === protótipos das funções ===

/**
 * inicialização
 * abre o arquivo da b-tree. se não existir, cria e inicializa cabeçalho/raiz
 */
FILE* btree_open(const char* filename);

/**
 * inserção
 * adiciona um novo registro de imagem ao índice. lida com divisão de nós e crescimento da raiz
 * retorna 1 em sucesso, 0 em falha
 */
int btree_insert(FILE* treeFile, IndexRecord record);

/**
 * busca
 * procura por um nome de arquivo na b-tree
 * retorna 1 se encontrou (preenchendo a struct *result), 0 se não encontrou
 */
int btree_search(FILE* treeFile, const char* key, int threshold, IndexRecord* foundRecord);

/**
 * debug / visualização
 * imprime a estrutura da árvore no console (útil para ver hierarquia)
 */
void btree_print_all(FILE* treeFile);

/**
 * limpeza
 * fecha o arquivo da b-tree
 */
void btree_close(FILE* treeFile);

/**
 * comparação de registros
 * compara nome e limiar de dois registros
 */
int compare_records(const char* keyName, int keyThreshold, IndexRecord existingRecord);

/**
 * deleção preguiçosa
 * marca um registro como deletado sem removê-lo fisicamente
 */
int btree_delete(FILE* treeFile, const char* key, int threshold);

/**
 * compactação do banco de dados
 * remove registros deletados e reconstrói a b-tree
 * @param binaryDataFile caminho para o arquivo de dados binários
 * @param btreeIndexFile caminho para o arquivo de índice b-tree
 * @return número de registros copiados, ou -1 em erro
 */
int btree_compact_database(const char *binaryDataFile, const char *btreeIndexFile);

/**
 * análise de fragmentação
 * analisa fragmentação no banco de dados
 * @param treeFile handle do arquivo b-tree aberto
 * @return número de registros deletados encontrados, ou -1 em erro
 */
int btree_analyze_fragmentation(FILE *treeFile);

/**
 * coleta de registros válidos
 * coleta todos os registros válidos (não deletados) da b-tree
 * @param treeFile handle do arquivo b-tree aberto
 * @param totalRecords parâmetro de saída para número de registros encontrados
 * @return array de structs IndexRecord (chamador deve liberar), ou NULL em erro
 */
IndexRecord* btree_collect_all_records(FILE *treeFile, int *totalRecords);

#endif