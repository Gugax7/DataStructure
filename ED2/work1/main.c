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
   inserir <arquivo.pgm> <limiar> - Insere uma imagem PGM (P2 ou P5) com um limiar.
   exportar <nome> <limiar>       - Exporta uma imagem do banco para PGM (ASCII P2).
   remover <nome> <limiar>        - Marca uma imagem para remocao.
   listar                         - Lista todas as imagens ativas no banco.
   compactar                      - Remove permanentemente as imagens marcadas.
   reconstruir <nome>             - Gera uma media de todas as versoes de uma imagem.
   ajuda                          - Mostra esta mensagem.

## Breve explicação de cada funcionalidade:

- Inserir: insere uma imagem compactada e limiarizada no arquivo database.bin
- Exportar: descompacta e cria uma copia da imagem na pasta do programa
- Listar: lista as imagens que estão salvas no database.bin (e não estão marcadas como removidas)
- Remover: coloca uma flag de removido na imagem que desejar (mas não a exclui)
- Compactar: exclui permanentemente os arquivos marcados como removidos do database.bin
- Reconstruir: pega todas as imagens com mesmo nome do banco de dados e realiza a média aritmética de cada pixel, gerando uma imagem mais proxima da original
    - a ideia é que quanto mais imagens houver no banco com a mesma origem, mais proxima ela estará da imagem que originou-as


acho que é isso, espero que goste :D

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DATABASE "database.bin"
#define TEMP_DATABASE "database_temp.bin"
#define INDEX_FILE "index.txt"
#define TEMP_INDEX_FILE "index_temp.txt"

typedef struct {
    int imgHeight, imgWidth, maxGrayValue;
    short int* pixelData;
} BinaryImage;

typedef struct {
    char recordName[256];
    int thresholdValue;
    long dataOffset;
    long blockSize;
    int isDeleted;
} IndexRecord;

// PROTOTIPOS

// usada no main pra simplificar o menu (e não colocar tudo em main)
void displayHelp(const char* programName);

// prototipos de funções principais relacionadas ao banco de imagens
int storeCompressedImage(const char *pgmFileName, int thresholdLevel, const char *binaryDataFile, const char *indexFileHandle);
int exportRecordToPgm(const char *nameToExport, int thresholdToExport, const char *binaryDataFile, const char *indexFileHandle);
int flagEntryForDeletion(const char *indexFileHandle, const char *nameToFlag, int thresholdToFlag);
void displayImageList(const char *indexFileHandle);
int compactDatabase(const char *binaryDataFile, const char *indexFileHandle);
int generateAverageImage(const char *baseName, const char *binaryDataFile, const char *indexFileHandle);

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
        compactDatabase(DATABASE, INDEX_FILE);
    }
    else if (strcmp(command, "reconstruir") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Erro: 'reconstruir' requer <nome>\n");
            fprintf(stderr, "Ex: %s reconstruir minha_imagem.pgm\n", argv[0]);
            return 1;
        }
        char *fileName = argv[2];
        generateAverageImage(fileName, DATABASE, INDEX_FILE);
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
 printf("  reconstruir <nome>             - Gera uma media de todas as versoes de uma imagem.\n");
 printf("  ajuda                          - Mostra esta mensagem.\n\n");
 printf("Arquivos de dados:\n");
 printf("  Indice: %s\n", INDEX_FILE);
 printf("  Dados: %s\n", DATABASE);
}

// (função inserir) armazena imagem comprimida com rle após limiar.
int storeCompressedImage(const char *pgmFileName, int thresholdLevel, const char *binaryDataFile, const char *indexFileHandle) 
{
 FILE *pgmInStream = fopen(pgmFileName, "rb"); // abre a imagem pgm (leitura binária)
 if (!pgmInStream) { perror("Erro ao abrir PGM"); return 0; } // checa abertura

 BinaryImage image = loadPgmFile(pgmInStream); // carrega o pgm para a struct de imagem
 
 fclose(pgmInStream); // fecha o pgm, não é mais necessário
 
 if (!image.pixelData) { fprintf(stderr, "Erro ao ler imagem.\n"); return 0; } // checa se a leitura dos pixels falhou

 applyThreshold(&image, thresholdLevel); // aplica o limiar (preto/branco)

 int runCount = 0;
 unsigned char startingValue = 0;
 int *runLengthArray = compressToRle(&image, &runCount, &startingValue); // comprime para rle, atualiza runcount por referência

 if (!runLengthArray && runCount > 0) { free(image.pixelData); fprintf(stderr, "Erro ao codificar RLE.\n"); return 0; } // checa falha na compressão rle

 FILE *binOutStream = fopen(binaryDataFile, "ab+"); // abre o arquivo de dados (append binary)
 if (!binOutStream) { perror("Erro ao abrir bin"); free(image.pixelData); free(runLengthArray); return 0; } // checa abertura do binário, limpa memória se falhar

 fseek(binOutStream, 0, SEEK_END); // vai para o fim do arquivo de dados
 long currentOffset = ftell(binOutStream); // pega o offset (endereço) de início deste registro

 // escreve os metadados no arquivo binário:
 fwrite(&image.imgWidth, sizeof(int), 1, binOutStream); // -> largura
 fwrite(&image.imgHeight, sizeof(int), 1, binOutStream); // -> altura
 fwrite(&image.maxGrayValue, sizeof(int), 1, binOutStream); // -> valor máximo de cinza
 fwrite(&startingValue, sizeof(unsigned char), 1, binOutStream); // -> bit inicial (0 ou 1)
 fwrite(&runCount, sizeof(int), 1, binOutStream); // -> contagem de 'corridas' (runs)
 if (runCount > 0) fwrite(runLengthArray, sizeof(int), runCount, binOutStream); // escreve o array rle (os dados)

 long compressedBytes = ftell(binOutStream) - currentOffset; // calcula o tamanho total em bytes deste registro
 fclose(binOutStream); // fecha o arquivo de dados.

 FILE *idxOutStream = fopen(indexFileHandle, "a"); // abre o arquivo de índice (append text)
 if (!idxOutStream) { perror("Erro abrir index"); free(image.pixelData); free(runLengthArray); return 0; } // checa abertura do índice.
 
 // escreve a nova linha no arquivo de índice
 // formato: nome limiar offset tamanho flag_removido(0)
 fprintf(idxOutStream, "%s %d %ld %ld %d\n", pgmFileName, thresholdLevel, currentOffset, compressedBytes, 0);
 fclose(idxOutStream); // fecha o índice.

 long originalPixelBytes = (long)image.imgWidth * (long)image.imgHeight * sizeof(short int); // calcula tamanho original para estatística

 free(image.pixelData); // libera memória da imagem original
 free(runLengthArray); // libera memória do array rle
 
 // imprime estatísticas para o usuário
 printf("Imagem '%s' (limiar=%d) salva no banco.\n", pgmFileName, thresholdLevel);
 printf(" -> Tamanho original (pixels em memoria): %ld bytes\n", originalPixelBytes);
 printf(" -> Tamanho comprimido (RLE + metadata): %ld bytes\n", compressedBytes);
 
 return 1; // retorna sucesso
}

// (função exportar) encontra um registro e salva como pgm (p2 ascii).
int exportRecordToPgm(const char *nameToExport, int thresholdToExport, const char *binaryDataFile, const char *indexFileHandle) {
 IndexRecord foundEntry = findRecordEntry(indexFileHandle, nameToExport, thresholdToExport); // procura o registro no índice
 if (foundEntry.dataOffset < 0) { printf("Registro nao encontrado.\n"); return 0; } // checa se não foi encontrado
 if (foundEntry.isDeleted) { printf("Registro marcado como removido.\n"); return 0; } // checa se está marcado como removido

 FILE *binInStream = fopen(binaryDataFile, "rb"); // abre o arquivo de dados (leitura binária)
 if (!binInStream) { perror("Erro abrir data file"); return 0; } // checa abertura

 BinaryImage exportedImage = loadRecordFromBinary(binInStream, foundEntry.dataOffset); // carrega (e descomprime) a imagem usando o offset
 fclose(binInStream); // fecha o arquivo de dados
 if (!exportedImage.pixelData) { printf("Erro ao decodificar registro.\n"); return 0; } // checa falha na decodificação

 char outputFileName[512]; // buffer nome de saída
 char baseFileName[256]; // buffer nome base
 strcpy(baseFileName, nameToExport); // copia nome
 
 char *dotPointer = strstr(baseFileName, ".pgm"); // procura por .pgm
 if(dotPointer) *dotPointer = '\0'; // ...e remove a extensão (se existir)

 snprintf(outputFileName, sizeof(outputFileName), "saida_%s_l%d.pgm", baseFileName, thresholdToExport); // formata o nome do arquivo de saída
 storePgmFile(outputFileName, &exportedImage); // salva a imagem da memória em um arquivo pgm
 free(exportedImage.pixelData); // libera a imagem descomprimida
 printf("Exportada em %s\n", outputFileName); // avisa o usuário
 return 1; // sucesso
}

// (função remover) marca uma entrada no índice (flag 1).
// (obs: não apaga os dados, só marca)
int flagEntryForDeletion(const char *indexFileHandle, const char *nameToFlag, int thresholdToFlag) 
{
 FILE *idxInStream = fopen(indexFileHandle, "r"); // abre o índice original (leitura)
 if (!idxInStream) { perror("Erro abrir index para remocao"); return 0; }

 FILE *tempIdxStream = fopen(TEMP_INDEX_FILE, "w"); // abre um índice temporário (escrita)
 if (!tempIdxStream) { perror("Erro criar index temporario"); fclose(idxInStream); return 0; } // checa abertura do temporário

 char currentName[256]; int currentThreshold; long currentOffset, currentSize; int currentFlag;
 int wasFound = 0; // flag para marcar se o registro foi encontrado

 // lê o índice original linha por linha
 while (fscanf(idxInStream, "%255s %d %ld %ld %d", currentName, &currentThreshold, &currentOffset, &currentSize, &currentFlag) == 5) {
  // checa se é o registro alvo (e não está marcado)
  if (!wasFound && strcmp(currentName, nameToFlag) == 0 && currentThreshold == thresholdToFlag && currentFlag == 0) {
   currentFlag = 1; // marca a flag como 1 (removido)
   wasFound = 1; // avisa que já foi encontrado
  }
  // escreve a linha (original ou modificada) no temporário
  fprintf(tempIdxStream, "%s %d %ld %ld %d\n", currentName, currentThreshold, currentOffset, currentSize, currentFlag);
 }
 fclose(idxInStream); fclose(tempIdxStream); // fecha ambos os arquivos

 if (!wasFound){ // se não foi encontrado...
 
  remove(TEMP_INDEX_FILE); // ...remove o temporário
  return 0; // não encontrado
 }

 // se foi encontrado:
 if (remove(indexFileHandle) != 0) perror("Aviso: nao foi possivel remover indice antigo"); // apaga o índice antigo...
 if (rename(TEMP_INDEX_FILE, indexFileHandle) != 0) perror("Aviso: nao foi possivel renomear indice"); // ...e renomeia o temporário para o nome original.
 return 1; // sucesso
}

// (função listar) lista imagens ativas (flag 0) no console.
void displayImageList(const char *indexFileHandle) {
 FILE *idxInStream = fopen(indexFileHandle, "r"); // abre o índice (leitura)
 if (!idxInStream) {
  perror("Erro ao abrir indice. O banco pode estar vazio"); // se falhar, o banco pode estar vazio
  return;
 }

 IndexRecord entry; // struct para leitura da linha
 int activeRecordCount = 0; // contador de registros ativos
 
 // imprime o cabeçalho
 printf("\n--- Imagens Ativas no Banco (%s) ---\n", indexFileHandle);
 printf("-------------------------------------------------------------------------\n");
 printf("%-30s | %-10s | %-12s | %-12s\n", "Nome", "Limiar", "Offset", "Tamanho (bytes)");
 printf("-------------------------------------------------------------------------\n");

 // lê o índice linha por linha
 while (fscanf(idxInStream, "%255s %d %ld %ld %d", 
    entry.recordName, &entry.thresholdValue, 
    &entry.dataOffset, &entry.blockSize, 
    &entry.isDeleted) == 5) {
  if (entry.isDeleted == 0) // se a flag for 0 (ativo)...
  {
   // ...imprime a linha formatada
   printf("%-30s | %-10d | %-12ld | %-12ld\n", 
     entry.recordName, entry.thresholdValue, 
     entry.dataOffset, entry.blockSize);
   activeRecordCount++; // ...e incrementa o contador
  }
 }
 
 // imprime o rodapé com o total
 printf("-------------------------------------------------------------------------\n");
 printf("Total de registros ativos: %d\n\n", activeRecordCount);
 fclose(idxInStream); // fecha o índice
}

// (função compactar) remove permanentemente registros com flag 1.
// recria o índice e o arquivo de dados sem os registros marcados.
int compactDatabase(const char *binaryDataFile, const char *indexFileHandle) 
{
 FILE *idxInStream = fopen(indexFileHandle, "r"); // abre índice original (leitura)
 if (!idxInStream) { perror("Erro abrir indice"); return -1; }

 FILE *binInStream = fopen(binaryDataFile, "rb"); // abre dados originais (leitura binária)
 if (!binInStream) { fclose(idxInStream); perror("Erro abrir datafile"); return -1; }

 FILE *tempDataStream = fopen(TEMP_DATABASE, "wb"); // cria novo arquivo de dados temporário (escrita binária)
 if (!tempDataStream) { fclose(idxInStream); fclose(binInStream); perror("Erro criar data temporario"); return -1; }
 
 FILE *tempIdxStream = fopen(TEMP_INDEX_FILE, "w"); // cria novo arquivo de índice temporário (escrita)
 if (!tempIdxStream) { fclose(idxInStream); fclose(binInStream); fclose(tempDataStream); perror("Erro criar index temporario"); return -1; }

 char currentName[256]; int currentThreshold; long currentOffset, currentSize; int currentFlag; // variáveis de leitura do índice
 long newOffset = 0; // offset no *novo* arquivo de dados
 int recordsCopied = 0; // contador de registros copiados

 const size_t BUF_SZ = 8192; // buffer de cópia (8kb)
 char *copyBuffer = malloc(BUF_SZ); // aloca o buffer de cópia
 if (!copyBuffer) { perror("Sem memoria para buffer"); fclose(idxInStream); fclose(binInStream); fclose(tempDataStream); fclose(tempIdxStream); return -1; }

 // lê o índice antigo linha por linha
 while (fscanf(idxInStream, "%255s %d %ld %ld %d", currentName, &currentThreshold, &currentOffset, &currentSize, &currentFlag) == 5) {
  if (currentFlag == 1) continue; // se flag=1 (removido), pula

  // se flag=0, copia o registro
  fseek(binInStream, currentOffset, SEEK_SET); // busca o offset no arquivo de dados *antigo*
  long remainingBytes = currentSize; // total de bytes a copiar para este registro

  // loop de cópia em pedaços (chunks)
  while (remainingBytes > 0) {
   size_t bytesToRead = (remainingBytes > (long)BUF_SZ) ? BUF_SZ : (size_t)remainingBytes; // calcula o tamanho do pedaço a ler
   size_t bytesRead = fread(copyBuffer, 1, bytesToRead, binInStream); // lê o pedaço do arquivo antigo...
   if (bytesRead == 0) break; // se a leitura falhar, para
   fwrite(copyBuffer, 1, bytesRead, tempDataStream); // ...e escreve no novo arquivo de dados (temporário)
   remainingBytes -= bytesRead; // atualiza bytes restantes
  }
  
  long newSize = currentSize; // o tamanho do bloco não muda
  fprintf(tempIdxStream, "%s %d %ld %ld %d\n", currentName, currentThreshold, newOffset, newSize, 0); // escreve a entrada no *novo* índice, com o *novo* offset
  newOffset += newSize; // atualiza o novo offset para o próximo registro
  recordsCopied++; // incrementa contador
 }

 free(copyBuffer); // libera o buffer de cópia
 fclose(idxInStream); fclose(binInStream); fclose(tempDataStream); fclose(tempIdxStream); // fecha todos os arquivos

 // substitui os arquivos antigos pelos novos
 if (remove(binaryDataFile) != 0) perror("Aviso: nao foi possivel remover data antigo");
 if (rename(TEMP_DATABASE, binaryDataFile) != 0) perror("Aviso: nao foi possivel renomear data");
 if (remove(indexFileHandle) != 0) perror("Aviso: nao foi possivel remover index antigo");
 if (rename(TEMP_INDEX_FILE, indexFileHandle) != 0) perror("Aviso: nao foi possivel renomear index");

 printf("Compactacao concluida. Registros copiados: %d\n", recordsCopied);
 return recordsCopied; // retorna o total de registros copiados
}

// (função reconstruir) encontra todas as versões (limiares) de uma imagem.
// gera uma nova imagem pgm com a média de pixels de todas as versões.
int generateAverageImage(const char *baseName, const char *binaryDataFile, const char *indexFileHandle) 
{
 FILE *idxInStream = fopen(indexFileHandle, "r"); // abre o índice
 if (!idxInStream) { perror("Erro abrir indice"); return 0; }

 typedef struct OffsetEntry { long offset; } OffsetEntry; // array dinâmico para guardar os offsets das versões
 OffsetEntry *offsetList = NULL; 
 int currentCapacity = 0, versionCount = 0; // controle do array dinâmico
 
 char currentName[256]; int currentThreshold; long currentOffset, currentSize; int currentFlag; // variáveis de leitura do índice
 int expectedWidth = -1, expectedHeight = -1, expectedGrayMax = -1; // variáveis para checar consistência de tamanho

 // encontra todos os offsets das versões ativas
 while (fscanf(idxInStream, "%255s %d %ld %ld %d", currentName, &currentThreshold, &currentOffset, &currentSize, &currentFlag) == 5) {
  if (strcmp(currentName, baseName) != 0) continue; // checa o nome base
  if (currentFlag == 1) continue; // checa se está removido (flag 1)
  
  // lógica de realocação do array de offsets (dobra a capacidade)
  if (versionCount+1 > currentCapacity) { 
   currentCapacity = currentCapacity ? currentCapacity*2 : 8; 
   offsetList = realloc(offsetList, currentCapacity * sizeof(OffsetEntry)); 
  }
  offsetList[versionCount++].offset = currentOffset; // salva o offset da versão encontrada
 }

 fclose(idxInStream); // fecha o índice
 if (versionCount == 0) { printf("Nenhuma versao encontrada para '%s'.\n", baseName); free(offsetList); return 0; } // se não achou versões, retorna

 // abre o arquivo de dados
 FILE *binInStream = fopen(binaryDataFile, "rb");
 if (!binInStream) { perror("Erro abrir datafile"); free(offsetList); return 0; }

 // carrega a primeira imagem (para referência e início da soma)
 BinaryImage firstImage = loadRecordFromBinary(binInStream, offsetList[0].offset); // carrega (descomprime) a primeira versão
 if (!firstImage.pixelData) { fclose(binInStream); free(offsetList); printf("Erro ao decodificar primeira versao.\n"); return 0; }
 
 expectedWidth = firstImage.imgWidth; expectedHeight = firstImage.imgHeight; expectedGrayMax = firstImage.maxGrayValue; // define as dimensões esperadas (w, h, graymax)
 long pixelCount = (long)expectedWidth * expectedHeight; // total de pixels

 long *pixelSumAccumulator = (long*)calloc(pixelCount, sizeof(long)); // aloca um acumulador (array de long) para a soma dos pixels
 if (!pixelSumAccumulator) { free(firstImage.pixelData); fclose(binInStream); free(offsetList); printf("Sem memoria para acumulador.\n"); return 0; }

 // soma os pixels da *primeira* imagem ao acumulador
 long *accumulatorPtr = pixelSumAccumulator;
 short int *pixelPtr = firstImage.pixelData;
 short int *endPtr = firstImage.pixelData + pixelCount;
 for (; pixelPtr < endPtr; pixelPtr++, accumulatorPtr++) {
  *accumulatorPtr += *pixelPtr;
 }
 free(firstImage.pixelData); // libera a memória da primeira imagem

 // carrega e soma as *outras* versões (a partir da segunda)
 for (int versionIndex = 1; versionIndex < versionCount; versionIndex++) {
  BinaryImage currentImage = loadRecordFromBinary(binInStream, offsetList[versionIndex].offset); // carrega (descomprime) a versão atual
  if (!currentImage.pixelData) { printf("Aviso: falha ao decodificar uma versao (offset %ld), pulando.\n", offsetList[versionIndex].offset); continue; } // se falhar ao decodificar, pula esta versão
  if (currentImage.imgWidth != expectedWidth || currentImage.imgHeight != expectedHeight) { printf("Aviso: dimensoes diferentes na versao offset %ld, pulando.\n", offsetList[versionIndex].offset); free(currentImage.pixelData); continue; } // se as dimensões forem inconsistentes, pula esta versão
  
  accumulatorPtr = pixelSumAccumulator; // reseta os ponteiros para o loop de soma
  pixelPtr = currentImage.pixelData;
  endPtr = currentImage.pixelData + pixelCount;
  
  // loop para somar os pixels da versão atual
  for (; pixelPtr < endPtr; pixelPtr++, accumulatorPtr++) {
   *accumulatorPtr += *pixelPtr;
  }
  free(currentImage.pixelData); // libera a memória da versão atual
 }

 // calcula a média e cria a imagem final
 BinaryImage resultImage = {expectedHeight, expectedWidth, expectedGrayMax, NULL}; // prepara a struct da imagem resultante
 resultImage.pixelData = (short int*)malloc(pixelCount * sizeof(short int)); // aloca memória para os pixels da imagem média
 if (!resultImage.pixelData) { printf("Sem memoria para imagem media.\n"); free(pixelSumAccumulator); fclose(binInStream); free(offsetList); return 0; }

 accumulatorPtr = pixelSumAccumulator; // loop pelo acumulador (soma total)
 long *accEndPtr = pixelSumAccumulator + pixelCount;
 short int *resultPtr = resultImage.pixelData;

 for (; accumulatorPtr < accEndPtr; accumulatorPtr++, resultPtr++) {
  long averageValue = *accumulatorPtr / versionCount; // calcula o valor médio do pixel
  
  // 'clampa' o valor (garante 0 <= valor <= graymax)
  if (averageValue < 0) averageValue = 0; 
  if (averageValue > expectedGrayMax) averageValue = expectedGrayMax;
  *resultPtr = (short int)averageValue; // salva o pixel médio na imagem resultante
 }

 // salva a imagem final em pgm
 char outputFileName[512]; 
 snprintf(outputFileName, sizeof(outputFileName), "reconstruida_media_%s", baseName); // formata o nome do arquivo de saída
 storePgmFile(outputFileName, &resultImage); // salva o pgm (p2 ascii)

 // limpa toda a memória alocada
 free(resultImage.pixelData); free(pixelSumAccumulator); fclose(binInStream); free(offsetList);
 printf("Reconstrucao media escrita em %s (usadas %d versoes).\n", outputFileName, versionCount);
 return 1; // sucesso
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