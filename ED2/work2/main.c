/*
ALUNO: GUSTAVO SALMAZO DA SILVA

# Algumas considerações sobre a utilização do programa:

- para compilar gcc main.c btree.c -o main
- ao executar, ele mostrará as opções de comando possiveis do programa
- nele é possivel: inserir, exportar, listar, e remover imagens limiarizadas
- o programa também permite compactação (hard remove) de banco de dados
- não é necessaria a criação de nenhum arquivo utilizado pelo programa (exemplo: database.bin), ele cria automaticamente quando necessario

## Utilização
- as imagens devem estar na mesma pasta que o programa (conforme enviado com a imagem exemplo (barbara.pgm))
- ao exportar imagens elas virão nomeadas como ("saida_<nome do arquivo>_l<limiar>.pgm")
- ao reconstruir imagens elas serão exportadas para a mesma pasta como "reconstruida_media_<nome da image>" (como elas são salvas como pgm, eu me aproveitei disso para não precisar escrever.pgm no fim)
- ao importar uma imagem é importante escrever exatamente o nome dela (com .pgm) como está salvo na pasta.

 Comandos:
   inserir <arquivo.pgm> <limiar> - Insere uma imagem PGM (P2 ou P5) com um limiar.
   exportar <nome> <limiar>       - Exporta uma imagem do banco para PGM (ASCII P2).
   remover <nome> <limiar>        - Marca uma imagem para remocao.
   listar                         - Lista todas as imagens ativas no banco.
   compactar                      - Remove permanentemente as imagens marcadas.
   ajuda                          - Mostra esta mensagem.

## Breve explicação de cada funcionalidade:

- Inserir: insere uma imagem compactada e limiarizada no arquivo database.bin (agora em ordem alfabetica + ordenada pelo limiar)
- Exportar: descompacta e cria uma copia da imagem na pasta do programa
- Listar: lista as imagens que estão salvas no database.bin (e não estão marcadas como removidas)
- Remover: coloca uma flag de removido na imagem que desejar (mas não a exclui)
- Compactar: exclui permanentemente os arquivos marcados como removidos do database.bin

Notas: 

- removi o sistema de restaurar imagens pois estava dando muitos bugs, e como não
  era requisito desse trabalho decidi remover pelo bem de minha sanidade mental
- como utilizei o trabalho 1 como molde aqui, muitas coisas estão bem parecidas,
  no caso foram mais alterações na forma de leitura/inserção/exportação
- não fiz o sistema de remoção adicional, como constava no slide, apenas um sistema simples
  como o professor sugeriu em seu ultimo comentario

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "btree.h"

#define TEMP_DATABASE "database_temp.bin"
#define DATABASE "data.bin"
#define INDEX_FILE "index.dat"
#define TEMP_INDEX_FILE "index_temp.txt"

typedef struct {
    int imgHeight, imgWidth, maxGrayValue;
    short int* pixelData;
} BinaryImage;


// PROTOTIPOS

// usada no main pra simplificar o menu (e não colocar tudo em main)
void displayHelp(const char* programName);

// prototipos de funções principais relacionadas ao banco de imagens
int storeCompressedImage(const char *pgmFileName, int thresholdLevel, const char *binaryDataFile, const char *indexFileHandle);
int exportRecordToPgm(const char *nameToExport, int thresholdToExport, const char *binaryDataFile, const char *indexFileHandle);
int flagEntryForDeletion(const char *indexFileHandle, const char *nameToFlag, int thresholdToFlag);
void displayImageList(const char *indexFileHandle);

// prototipos de funções utilizadas internamente no banco de imagens
IndexRecord findRecordEntry(const char *indexFileHandle, const char *searchName, int searchThreshold);
BinaryImage loadRecordFromBinary(FILE *binInStream, long dataOffset);
BinaryImage loadPgmFile(FILE* pgmStream);
void storePgmFile(const char *destinationFile, BinaryImage *imageToStore);
void applyThreshold(BinaryImage *imageToProcess, int thresholdLevel);
int* compressToRle(BinaryImage *inputImage, int *runCountOutput, unsigned char *firstPixelValue);
BinaryImage expandFromRle(int w, int h, int gray, unsigned char startingValue, int *runLengthArray, int runCount);


int main(int argc, char *argv[]) {
    if (argc < 2) {
        displayHelp(argv[0]);
        return 1;
    }

    char *command = argv[1];

    if (strcmp(command, "inserir") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erro: 'inserir' requer <arquivo.pgm> e <limiar>\n");
            fprintf(stderr, "Ex: %s inserir minha_imagem.pgm 128\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdValue = atoi(argv[3]);
        if (thresholdValue < 0 || thresholdValue > 255) {
             fprintf(stderr, "Erro: Limiar deve estar entre 0 e 255.\n");
             return 1;
        }
        storeCompressedImage(fileName, thresholdValue, DATABASE, INDEX_FILE);
    }
    else if (strcmp(command, "exportar") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erro: 'exportar' requer <nome> e <limiar>\n");
            fprintf(stderr, "Ex: %s exportar minha_imagem.pgm 128\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdValue = atoi(argv[3]);
        exportRecordToPgm(fileName, thresholdValue, DATABASE, INDEX_FILE);
    }
    else if (strcmp(command, "remover") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Erro: 'remover' requer <nome> e <limiar>\n");
            fprintf(stderr, "Ex: %s remover minha_imagem.pgm 128\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        int thresholdValue = atoi(argv[3]);
        if (flagEntryForDeletion(INDEX_FILE, fileName, thresholdValue)) {
            printf("Imagem '%s' (limiar %d) marcada como removida.\n", fileName, thresholdValue);
        } else {
            printf("Imagem '%s' (limiar %d) nao encontrada ou ja removida.\n", fileName, thresholdValue);
        }
    }
    else if (strcmp(command, "compactar") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Erro: 'compactar' nao requer argumentos.\n");
            return 1;
        }
         btree_compact_database(DATABASE, INDEX_FILE);
    }
    else if (strcmp(command, "listar") == 0) {
         if (argc != 2) {
            fprintf(stderr, "Erro: 'listar' nao requer argumentos.\n");
            return 1;
        }
        displayImageList(INDEX_FILE);
    }
    else if (strcmp(command, "ajuda") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        displayHelp(argv[0]);
    }
    else {
        fprintf(stderr, "Comando desconhecido: '%s'\n", command);
        displayHelp(argv[0]);
        return 1;
    }

    return 0;
}

void displayHelp(const char* programName) {
 printf("Uso: %s <comando> [argumentos]\n\n", programName);
 printf("Comandos:\n");
 printf("  inserir <arquivo.pgm> <limiar> - Insere uma imagem PGM (P2 ou P5) com um limiar.\n");
 printf("  exportar <nome> <limiar>       - Exporta uma imagem do banco para PGM (ASCII P2).\n");
 printf("  remover <nome> <limiar>        - Marca uma imagem para remocao.\n");
 printf("  listar                         - Lista todas as imagens ativas no banco.\n");
 printf("  compactar                      - Remove permanentemente as imagens marcadas.\n");
 printf("  ajuda                          - Mostra esta mensagem.\n\n");
 printf("Arquivos de dados:\n");
 printf("  Indice: %s\n", INDEX_FILE);
 printf("  Dados: %s\n", DATABASE);
}

int storeCompressedImage(const char *pgmFileName, int thresholdLevel, const char *binaryDataFile, const char *btreeFileName)
{ 
    // abre o arquivo pgm para leitura
    FILE *pgmInStream = fopen(pgmFileName, "rb");
    if (!pgmInStream) { 
        perror("Erro ao abrir PGM"); 
        return 0; 
    }
    
    // carrega a imagem do arquivo pgm
    BinaryImage image = loadPgmFile(pgmInStream);
    fclose(pgmInStream);
    
    // verifica se conseguiu carregar os dados da imagem
    if (!image.pixelData) { 
        fprintf(stderr, "Erro ao ler imagem (dados nulos).\n"); 
        return 0; 
    }
    
    // aplica o filtro de limiar (transforma em imagem binária)
    applyThreshold(&image, thresholdLevel);
    
    // comprime a imagem usando rle (run-length encoding)
    int runCount = 0;
    unsigned char startingValue = 0;
    int *runLengthArray = compressToRle(&image, &runCount, &startingValue);
    
    // verifica se a compressão funcionou
    if (!runLengthArray && runCount > 0) {
        free(image.pixelData);
        fprintf(stderr, "Erro ao codificar RLE.\n");
        return 0;
    }
    
    // abre o arquivo de dados em modo append (adiciona no final)
    FILE *binOutStream = fopen(binaryDataFile, "ab+");
    if (!binOutStream) {
        perror("Erro ao abrir arquivo de dados binários");
        free(image.pixelData);
        free(runLengthArray);
        return 0;
    }
    
    // vai para o final do arquivo e pega a posição (offset)
    fseek(binOutStream, 0, SEEK_END);
    long currentOffset = ftell(binOutStream);
    
    // escreve todos os dados da imagem comprimida em sequência:
    fwrite(&image.imgWidth, sizeof(int), 1, binOutStream);        // largura
    fwrite(&image.imgHeight, sizeof(int), 1, binOutStream);       // altura
    fwrite(&image.maxGrayValue, sizeof(int), 1, binOutStream); // valor máximo de cinza
    fwrite(&startingValue, sizeof(unsigned char), 1, binOutStream); // primeiro bit da compressão
    fwrite(&runCount, sizeof(int), 1, binOutStream);              // quantas "corridas" tem no rle
    
    // escreve o array com os tamanhos das corridas (se existir)
    if (runCount > 0) {
        fwrite(runLengthArray, sizeof(int), runCount, binOutStream);
    }
    
    // calcula quantos bytes foram escritos no total
    long compressedBytes = ftell(binOutStream) - currentOffset;
    fclose(binOutStream);
    
    // prepara o registro que vai ser inserido na b-tree
    IndexRecord newRecord;
    memset(&newRecord, 0, sizeof(IndexRecord)); // limpa a memória (evita lixo)
    
    // preenche os dados do registro:
    strncpy(newRecord.recordName, pgmFileName, sizeof(newRecord.recordName) - 1); // nome do arquivo
    newRecord.thresholdValue = thresholdLevel;  // limiar usado
    newRecord.dataOffset = currentOffset;       // onde os dados estão no arquivo binário
    newRecord.blockSize = compressedBytes;      // tamanho dos dados
    newRecord.isDeleted = 0;                    // marca como não deletado
    
    // abre a b-tree (cria se não existir)
    FILE *treeFile = btree_open(btreeFileName);
    if (!treeFile) {
        fprintf(stderr, "Erro ao abrir ou criar o arquivo da B-Tree.\n");
        free(image.pixelData);
        free(runLengthArray);
        return 0;
    }
    
    // tenta inserir o registro na b-tree
    int insertResult = btree_insert(treeFile, newRecord);
    btree_close(treeFile);
    
    // verifica se a inserção deu certo
    if (insertResult != 1) {
        fprintf(stderr, "Erro: Chave duplicada ou falha na inserção da B-Tree.\n");
    }
    
    // libera toda a memória que foi alocada
    free(image.pixelData);
    free(runLengthArray);
    
    // mostra mensagem de sucesso se tudo deu certo
    if (insertResult == 1) {
        printf("Sucesso: Imagem '%s' armazenada.\n", pgmFileName);
        printf(" -> Offset: %ld, Tamanho: %ld bytes\n", currentOffset, compressedBytes);
    }
    
    return insertResult; // retorna 1 se sucesso, 0 se falhou
}

// (função exportar) encontra um registro e salva como pgm (p2 ascii).
int exportRecordToPgm(const char *name, int threshold, const char *dbFile, const char *idxFile) {
    // abre o arquivo da arvore com a funcao auxiliar do btree.c
    FILE *tree = btree_open(idxFile);
    if (!tree) return 0;

    IndexRecord record;
    // aqui a maior mudança foi que ele utiliza o threshold para organizar a ordem além do nome
    if (!btree_search(tree, name, threshold, &record)) {
        printf("Erro: Imagem '%s' com limiar %d nao encontrada.\n", name, threshold);
        btree_close(tree);
        return 0;
    }
    btree_close(tree);

    // valida
    if (record.isDeleted) {
        printf("Erro: A imagem solicitada foi excluida.\n");
        return 0;
    }

    FILE *bin = fopen(dbFile, "rb");
    if (!bin) { perror("Erro ao abrir banco de dados"); return 0; }

    fseek(bin, record.dataOffset, SEEK_SET);

    int w, h, maxGray, runs;
    unsigned char startBit;
    
    // lÊ os metadados do arquivo
    fread(&w, sizeof(int), 1, bin);
    fread(&h, sizeof(int), 1, bin);
    fread(&maxGray, sizeof(int), 1, bin);
    fread(&startBit, sizeof(unsigned char), 1, bin);
    fread(&runs, sizeof(int), 1, bin);

    // le o arquivo (que contém informações comprimidas)
    int *rleData = (int*) malloc(runs * sizeof(int));
    if (!rleData) {
        perror("Erro de memoria ao ler RLE");
        fclose(bin);
        return 0;
    }
    fread(rleData, sizeof(int), runs, bin);
    fclose(bin);

    //chama a função de descompressão (expandFromRLE)
    printf("Descomprimindo imagem (%d x %d)...\n", w, h);
    BinaryImage reconstructedImg = expandFromRle(w, h, maxGray, startBit, rleData, runs);

    if (!reconstructedImg.pixelData) {
        printf("Erro critico: Falha na alocacao de memoria para pixels.\n");
        free(rleData);
        return 0;
    }

    char outName[256];
    // cria um novo nome de arquivo para não sobrescrever nada
    snprintf(outName, sizeof(outName), "export_%d_%s", threshold, name);
    
    storePgmFile(outName, &reconstructedImg);
    printf("Sucesso! Imagem exportada como: %s\n", outName);

    free(rleData);
    free(reconstructedImg.pixelData); // libera o espaço dos pixels que alocou
    
    return 1;
}

// (função remover) marca uma entrada no índice (flag 1).
// (obs: não apaga os dados, só marca)
int flagEntryForDeletion(const char *indexFileHandle, const char *nameToFlag, int thresholdToFlag) 
{
    // abre a b-tree
    FILE *tree = btree_open(indexFileHandle);
    if (!tree) {
        perror("Erro ao abrir indice para remocao");
        return 0;
    }

    // chama a funcao auxiliar para "deletar" o item
    int success = btree_delete(tree, nameToFlag, thresholdToFlag);

    btree_close(tree);

    // fala pra mim se deu bom
    if (success) {
        printf("Sucesso: Registro '%s' (Limiar %d) marcado como removido.\n", nameToFlag, thresholdToFlag);
        return 1;
    } else {
        printf("Erro: Registro '%s' (Limiar %d) nao encontrado ou ja removido.\n", nameToFlag, thresholdToFlag);
        return 0;
    }
}

// (função listar) lista imagens ativas (flag 0) no console.
void displayImageList(const char *idxFile) {
    FILE *tree = btree_open(idxFile);
    if (!tree) {
        printf("Indice nao encontrado.\n");
        return;
    }
    btree_print_all(tree);
    btree_close(tree);
}

// procura no índice por nome e limiar.
// retorna o registro. se não achar, retorna offset = -1.
IndexRecord findRecordEntry(const char *indexFileHandle, const char *searchName, int searchThreshold) 
{
 FILE *idxInStream = fopen(indexFileHandle, "r"); // abre o índice
 IndexRecord entry = {"", -1, -1, -1, 0}; // struct padrão de retorno (offset -1 = não encontrado)
 if (!idxInStream) return entry; // se falhar ao abrir, retorna 'não encontrado'

 // lê linha por linha
 while (fscanf(idxInStream, "%255s %d %ld %ld %d", 
     entry.recordName, &entry.thresholdValue, 
     &entry.dataOffset, &entry.blockSize, 
     &entry.isDeleted) == 5) {
  if ((strcmp(entry.recordName, searchName) == 0) && (entry.thresholdValue == searchThreshold)){ // compara nome e limiar
  
   fclose(idxInStream); // encontrado!
   return entry; // retorna o registro encontrado
  }
 }
 fclose(idxInStream); // fecha o arquivo
 entry.dataOffset = -1; // garante offset -1 se não foi encontrado
 return entry; // retorna 'não encontrado'
}

// lê um registro (metadados + rle) do arquivo binário.
// retorna a imagem descomprimida (pixeldata alocado).
BinaryImage loadRecordFromBinary(FILE *binInStream, long dataOffset) 
{
 BinaryImage loadedImage = {0,0,0,NULL}; // imagem de retorno em caso de erro
 if (!binInStream) return loadedImage; // checa stream
 fseek(binInStream, dataOffset, SEEK_SET); // busca o offset (posição inicial) do registro
 int w, h, gray; 
 unsigned char startingValue; int runCount;

 // lê os metadados (w, h, gray, bit inicial, contagem de runs)
 if (fread(&w, sizeof(int), 1, binInStream) != 1) return loadedImage;
 if (fread(&h, sizeof(int), 1, binInStream) != 1) return loadedImage;
 if (fread(&gray, sizeof(int), 1, binInStream) != 1) return loadedImage;
 if (fread(&startingValue, sizeof(unsigned char), 1, binInStream) != 1) return loadedImage;
 if (fread(&runCount, sizeof(int), 1, binInStream) != 1) return loadedImage;

 int *runLengthArray = NULL; // ponteiro para o array rle
 if (runCount > 0) // se houver dados rle (runcount > 0)...
 {
  runLengthArray = (int*)malloc(runCount * sizeof(int)); // ...aloca memória para o array rle
  if (!runLengthArray) return loadedImage; // checa alocação
  // ...lê o array rle do arquivo para a memória
  if (fread(runLengthArray, sizeof(int), runCount, binInStream) != (size_t)runCount) { free(runLengthArray); return loadedImage; } // falha na leitura do array rle
 }

 loadedImage = expandFromRle(w, h, gray, startingValue, runLengthArray, runCount); // expande o rle para os dados de pixel
 if (runLengthArray) free(runLengthArray); // libera o array rle (não é mais necessário)
 return loadedImage; // retorna a imagem descomprimida
}

// --- LEITURA E ESCRITA PGM ---

// lê um arquivo pgm (p2 ou p5) para a struct binaryimage.
BinaryImage loadPgmFile(FILE* pgmStream) {
 BinaryImage loadedImage = {0, 0, 0, NULL}; // imagem de retorno em caso de erro
 char imageFormat[3];
 int h, w, gray;

 if (fscanf(pgmStream, "%2s", imageFormat) != 1) return loadedImage; // lê o "número mágico" (p2 ou p5)
 if (strcmp(imageFormat, "P5") != 0 && strcmp(imageFormat, "P2") != 0) return loadedImage; // checa se o formato é p2 ou p5
 fgetc(pgmStream); // consome o '\n'

 // loop para ignorar comentários (#)
 char currentChar = fgetc(pgmStream);
 while (currentChar == '#'){ 
  while (fgetc(pgmStream) != '\n');
  currentChar = fgetc(pgmStream);
 }
 ungetc(currentChar, pgmStream); // devolve o caractere (não-'#') lido ao stream

 if (fscanf(pgmStream, "%d%d", &w, &h) != 2) return loadedImage; // lê largura (w) e altura (h)
 if (fscanf(pgmStream, "%d", &gray) != 1) return loadedImage; // lê o valor máximo de cinza
 fgetc(pgmStream); // consome o '\n' final do cabeçalho

 loadedImage.imgWidth = w; // guarda os metadados na struct
 loadedImage.imgHeight = h;
 loadedImage.maxGrayValue = gray;
 loadedImage.pixelData = (short int*)malloc(w * h * sizeof(short int)); // aloca memória para os pixels

 if(loadedImage.pixelData == NULL) return loadedImage; // checa alocação

 short int *pixelPtr = loadedImage.pixelData; // ponteiros para o loop de leitura
 short int *endPtr = loadedImage.pixelData + (w * h); 

 if (strcmp(imageFormat, "P5") == 0){ // se for p5 (binário)...
 
  // loop de leitura (1 byte por pixel)
  for (; pixelPtr < endPtr; pixelPtr++) {
  
   unsigned char readByte; // buffer de 1 byte
   if (fread(&readByte, sizeof(unsigned char), 1, pgmStream) != 1){ // lê 1 byte
   
    free(loadedImage.pixelData); // falha na leitura (imagem incompleta)
    loadedImage.pixelData = NULL;
    return loadedImage;
   }
   *pixelPtr = (short int)readByte; // salva o byte como 'short int' no array
  }
 } 
 else{ // se for p2 (ascii)...
  // loop de leitura (números como texto)
  for (; pixelPtr < endPtr; pixelPtr++) {
  
   if (fscanf(pgmStream, "%hd", pixelPtr) != 1){ // lê o número (texto) para o 'short int'
   
    free(loadedImage.pixelData); // falha na leitura (imagem incompleta)
    loadedImage.pixelData = NULL;
    return loadedImage;
   }
  }
 }
 return loadedImage; // retorna a imagem carregada
}

// salva uma binaryimage como pgm p2 (ascii).
void storePgmFile(const char *destinationFile, BinaryImage *imageToStore) 
{
 FILE* pgmOutStream = fopen(destinationFile, "wb"); // abre o arquivo de destino (escrita)
 if(pgmOutStream == NULL) {
 
  perror("Erro ao abrir arquivo para escrita PGM");
  return;
 }

 fprintf(pgmOutStream, "P2\n"); // escreve o cabeçalho pgm (p2 ascii)
 fprintf(pgmOutStream, "%d %d\n", imageToStore->imgWidth, imageToStore->imgHeight);
 fprintf(pgmOutStream, "%d\n", imageToStore->maxGrayValue);

 long pixelCount = (long)imageToStore->imgWidth * imageToStore->imgHeight; // total de pixels
 short int *pixelPtr = imageToStore->pixelData; // ponteiros para o loop
 short int *endPtr = imageToStore->pixelData + pixelCount;
 
 // loop por todos os pixels
 for (; pixelPtr < endPtr; pixelPtr++) {

  fprintf(pgmOutStream, "%d ", *pixelPtr); // escreve o valor do pixel (texto) e um espaço
 }

 fclose(pgmOutStream); // fecha o arquivo
}

// --- PROCESSAMENTO DE IMAGEM E RLE ---

// (processamento) aplica o limiar na imagem.
// (modifica os pixels da própria struct).
void applyThreshold(BinaryImage *imageToProcess, int thresholdLevel) 
{
 long pixelCount = (long)imageToProcess->imgWidth * imageToProcess->imgHeight; // total de pixels
 short int *pixelPtr = imageToProcess->pixelData; // ponteiros para o loop
 short int *endPtr = imageToProcess->pixelData + pixelCount;

 // loop por todos os pixels
 for (; pixelPtr < endPtr; pixelPtr++){ 
  if (*pixelPtr < thresholdLevel) // se for menor que o limiar...
   *pixelPtr = 0;    // ...vira 0 (preto)
  else // senão...
   *pixelPtr = imageToProcess->maxGrayValue; // ...vira o valor máximo (branco)
 }
}

// (processamento) comprime uma imagem limiarizada (0/maxgray) para rle.
// retorna array rle alocado; atualiza runcount e firstpixel (por ref).
// naquele esquema de [valor inicial, quantas vezes repete, quantas vezes o outro repete, quantas vezes inicial repete...] tipo -> [0,3,6,2] = 0 0 0 1 1 1 1 1 1 0 0
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

// (processamento) descomprime dados rle (metadados + array de runs).
// retorna uma binaryimage (com pixeldata alocado).
BinaryImage expandFromRle(int w, int h, int gray, unsigned char startingValue, int *runLengthArray, int runCount) 
{
 BinaryImage outputImage = {h, w, gray, NULL}; // prepara a struct da imagem de saída
 int pixelCount = w * h; // total de pixels
 outputImage.pixelData = (short int*)malloc(pixelCount * sizeof(short int)); // aloca memória para os pixels
 if (!outputImage.pixelData) return outputImage; // checa alocação

 short int *pixelPtr = outputImage.pixelData; // ponteiros
 short int *endPtr = outputImage.pixelData + pixelCount;
 unsigned char currentValue = startingValue; // valor (bit 0 ou 1) a ser escrito; começa com o 'startingvalue'

 // loop por cada 'corrida' (run) no array rle
 for (int rIndex = 0; rIndex < runCount; ++rIndex) {
  int runLength = runLengthArray[rIndex]; // pega o tamanho (duração) da 'corrida' atual
  
  // loop interno: escreve o valor runlength vezes
  for (int k = 0; k < runLength; ++k){ 
  
   if (pixelPtr >= endPtr) break; // checagem de segurança (evita buffer overflow)
   *pixelPtr = currentValue ? gray : 0; // converte o bit (0/1) de volta para pixel (0/graymax)
   pixelPtr++; // avança o ponteiro do pixel
  }
  currentValue = 1 - currentValue; // inverte o bit (0->1 ou 1->0) para a próxima 'corrida'
 }

 // (segurança) preenche o resto da imagem com 0 se o rle for inválido
 while (pixelPtr < endPtr) {
  *pixelPtr = 0;
  pixelPtr++;
 }
 return outputImage; // retorna a imagem descomprimida
}