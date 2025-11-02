#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <direct.h>

#define INDEX_FILE "index.bin"
#define DATABASE_FILE "database.bin"

typedef struct{
  int* values;
  int width, height;
  int grayMax;
  char config[3];
}PgmImage;

typedef struct{
  char fileName[256];
  long offset;
  long size;
}Index;

void convertImageToStruct(FILE *pgm, PgmImage *object){
  
  // type
  char config[3];
  int width, height;
  int maxGray;
  

  if (fscanf(pgm, "%s", config) != 1) {
    printf("Invalid PGM header\n");
    return;
  }

  if(config[0] != 'P' || config[1] != '2'){
    printf("Only P2 formats accepted in the moment\n");
    return;
  }

  strcpy(object->config, config);

  // jump comments
  char c;

  while((c = getc(pgm)) == '#'){
    while((c = fgetc(pgm)) != '\n');
  }

  ungetc(c, pgm);

  // size x size
  fscanf(pgm, "%d %d", &width, &height);
  // gray scale
  fscanf(pgm, "%d", &maxGray);

  // writing values
  object->width = width;
  object->height = height;
  object->grayMax = maxGray;
  object->values = (int *) malloc(sizeof(int) * width * height);
  if(object->values == NULL){
    printf("Fail to allocate memory to pgm values");
  }

  // values

  for(int i = 0; i < width * height; i++){
    fscanf(pgm, "%d", &object->values[i]);
  }
}

int importImage(char * pgmFile){

  FILE* pgmStream;
  FILE* databaseStream;
  FILE* indexStream;

  pgmStream = fopen(pgmFile, "r");
  databaseStream = fopen(DATABASE_FILE, "ab+");

  if(!pgmStream || !databaseStream) return -1;

  PgmImage imageObject;
  convertImageToStruct(pgmStream, &imageObject);

  fseek(databaseStream, 0, SEEK_END);

  long currentOffSet = ftell(databaseStream);

  // write to database

  fwrite(imageObject.config, sizeof(char), 3, databaseStream);
  fwrite(&imageObject.width, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.height, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.grayMax, sizeof(int), 1, databaseStream);
  fwrite(imageObject.values, sizeof(int), imageObject.height * imageObject.width, databaseStream);

  indexStream = fopen(INDEX_FILE, "ab+");

  // take size

  fseek(databaseStream, 0, SEEK_END);

  long size = ftell(databaseStream) - currentOffSet;

  // write to indexFile

  Index indexItem;
  memset(&indexItem, 0, sizeof(Index));
  strncpy(indexItem.fileName, pgmFile, sizeof(indexItem.fileName) - 1);
  indexItem.offset = currentOffSet;
  indexItem.size = size;

  fwrite(&indexItem, sizeof(Index), 1, indexStream);

  // close and free everyone

  fclose(pgmStream);
  fclose(databaseStream);
  fclose(indexStream);

  free(imageObject.values);
  
  return 0;
}

void printIndexFile(){
  FILE* indexStream = fopen(INDEX_FILE, "rb");

  Index indexItem;

  while(fread(&indexItem, sizeof(Index), 1, indexStream) == 1){
    printf("Index item:\n\tpgmName: %s\n\toffset: %ld\n\tsize: %ld\n", indexItem.fileName, indexItem.offset, indexItem.size);
  }
}

void thresholdImage(PgmImage image, int threshold){
  if(image.values == NULL){
    printf("Error accessing image values\n");
    return;
  }

  for(int i = 0; i < image.width * image.height; i++){
    if(image.values[i] > threshold) image.values[i] = image.grayMax;
    else image.values[i] = 0;
  }
}

void reverseImage(PgmImage image){
  if(image.values == NULL){
    printf("Error accessing image values\n");
    return;
  }

  for(int i = 0; i < image.width * image.height; i++){
    image.values[i] = image.grayMax - image.values[i];
  }
}


int exportImage(char * imageName, char *outputName, int exportMode, int threshold){
  FILE* outputStream = fopen(outputName, "w");
  FILE* indexStream = fopen(INDEX_FILE, "rb");

  if (!outputStream) {
    perror("Failed to open output file");
    return 1;
  }
  if (!indexStream) {
      perror("Failed to open index file");
      fclose(outputStream);
      return 1;
  }

  // first version the index was in txt, so for study i decided to keep it.
  // Find the index with this outputName (in text);
  // do{
  //   position++;
  //   if(fscanf(indexStream, "%s %ld %ld",inFileName, &offset, &size) != 3){
  //     return 1;
  //   };
  // }while(strcmp(inFileName, imageName) != 0);


  Index indexItem;
  memset(&indexItem, 0, sizeof(Index));

  int found = 0;

  while(fread(&indexItem, sizeof(Index), 1, indexStream) == 1){
    if(strcmp(indexItem.fileName, imageName) == 0){
      found = 1;
      break;
    }
  }

  if(!found){
    printf("file not found\n");
    return 1;
  }

  //printf("on position %d: found!, %s %ld %ld", position, inFileName, offset, size);

  // from here i actually have this images offsets, sizes and positions

  FILE* databaseStream = fopen(DATABASE_FILE, "rb");

  PgmImage outputObject;

  fseek(databaseStream, indexItem.offset, SEEK_SET);

  fread(outputObject.config, sizeof(char), 3, databaseStream);
  fread(&outputObject.width, sizeof(int), 1, databaseStream);
  fread(&outputObject.height, sizeof(int), 1, databaseStream);
  fread(&outputObject.grayMax, sizeof(int), 1, databaseStream);

  outputObject.values = (int*)malloc(sizeof(int) * outputObject.width * outputObject.height);
  if(outputObject.values == NULL){
    printf("it failed!\n");
    return 1;
  }

  fread(outputObject.values, sizeof(int), outputObject.width * outputObject.height, databaseStream);

  // right here i already have the output object, i just need to configure it to the current exporting mode

  switch (exportMode)
  {
  case 1:
    printf("threshold mode chosen: exporting image %s, with a threshold of %d\n", imageName, threshold);
    
    thresholdImage(outputObject, threshold);

    break;
  
  case 2:
    printf("reverse mode chosen: exporting image %s, with reversed pixels\n", imageName);

    reverseImage(outputObject);

    break;
  default:
    
    printf("default method chosen: exporting image %s as it was\n", imageName);

    break;
  }

  fprintf(outputStream, "%s\n", outputObject.config);
  fprintf(outputStream, "%d %d\n", outputObject.width, outputObject.height);
  fprintf(outputStream, "%d\n", outputObject.grayMax);

  for(int i = 0; i < outputObject.width * outputObject.height; i++){
    fprintf(outputStream, "%d ", outputObject.values[i]);
  }

  
  free(outputObject.values);
  fclose(outputStream);
  fclose(indexStream);
  fclose(databaseStream);

  return 0;
}

// my idea here is simple, first i start dividing all items in their own files, to guarantee that
// with only one, it will be sorted, so i can do the kway merge
int separateIndex(FILE* index){
  Index item;
  int x = 1;
  char fileName[256];
  char path[300];

  while(fread(&item, sizeof(Index), 1, index) == 1){
    sprintf(fileName, "part_%d.bin", x);
    sprintf(path, "./index_parts/%s", fileName);

    FILE *part_x = fopen(path, "wb");

    fwrite(&item, sizeof(Index), 1, part_x);
    x++;

    fclose(part_x);
  }

  return x;
}

// function to merge indexes two by two, as the mister asked
void mergeIndexes(FILE* index1, FILE* index2, int count){
  int one_finished = 0;
  int two_finished = 0;

  Index item1, item2;

  if(fread(&item1, sizeof(Index), 1, index1) != 1) one_finished = 1;
  if(fread(&item2, sizeof(Index), 1, index2) != 1) two_finished = 1;

  char fileName[256];

  sprintf(fileName, "./index_parts/part_%d.bin", count);

  FILE* newFile = fopen(fileName, "wb");

  // copy until someone get empty

  while(!one_finished && !two_finished){
    if(strcmp(item1.fileName, item2.fileName) <= 0){
      fwrite(&item1, sizeof(Index), 1, newFile);

      if(fread(&item1, sizeof(Index), 1, index1) != 1){
        one_finished = 1;
      }
    }else{
      fwrite(&item2, sizeof(Index), 1, newFile);

      if(fread(&item2, sizeof(Index), 1, index2) != 1){
        two_finished = 1;
      }
    }
  }

  // when one of them is empty, copy the other one entirely

  while(!one_finished){
    fwrite(&item1, sizeof(Index), 1, newFile);

    if(fread(&item1, sizeof(Index), 1, index1) != 1){
      one_finished = 1;
    }
  }

  while(!two_finished){
    fwrite(&item2, sizeof(Index), 1, newFile);

      if(fread(&item2, sizeof(Index), 1, index2) != 1){
        two_finished = 1;
      }
  }

  fclose(newFile);
}

void sortIndex(FILE* index){
  int initialPartsCount = separateIndex(index);
  int totalParts = initialPartsCount;
  int currentPart = 1;
  char path[256];

  while(currentPart + 1 < totalParts){
    sprintf(path, "./index_parts/part_%d.bin", currentPart);
    
    FILE *file1 = fopen(path, "rb");
    if(file1 == NULL){
      printf("Error opening file: %s", path);
      fclose(file1);  
      return;
    }

    currentPart++;
    
    sprintf(path, "./index_parts/part_%d.bin", currentPart);

    FILE *file2 = fopen(path, "rb");

    if(file2 == NULL){
      printf("Error opening file: %s", path);
      fclose(file1);
      fclose(file2);
      return;
    }

    currentPart++;

    mergeIndexes(file1, file2, totalParts++);

    fclose(file1);
    fclose(file2);
  }

  printf("total leaving loop: %d\n", totalParts);

  // take current index file and throw it into backup folder.
  
  time_t now = time(NULL);

  char backupName[256];
  char sortedFile[256];

  sprintf(backupName, "index_backup/index_%ld.bin", now);
  sprintf(sortedFile, "index_parts/part_%d.bin", (totalParts - 1));

  fclose(index);

  // rename last file to new index and saving old one as backup

  if(rename(INDEX_FILE, backupName) != 0){
    perror("rename failed");
    return;
  }
  if(rename(sortedFile, INDEX_FILE) != 0){
    perror("rename failed");
    rename(backupName, INDEX_FILE);
    return;
  }

  for(int i = 1; i < totalParts - 1; i++){
    char fileToRemove[256];
    sprintf(fileToRemove, "index_parts/part_%d.bin", i);
    if(remove(fileToRemove) != 0){
      perror("failed to remove file part index");
      return;
    }
  }

  // just an idea: what if i remove the index_parts folder every time?
  // i think that it just dont worth the risk haha

  // if(_rmdir("index_parts") != 0){
  //   perror("failed to remove parts folder");
  //   return;
  // }
  
}

void printExplanation(char *programName){
  printf("commands allowed:\n");
  printf("\t-%s import <file_name.pgm>\n", programName);
  printf("\t-%s export <file_name.pgm>\n", programName);
  printf("\t-%s export <file_name.pgm> reverse\n", programName);
  printf("\t-%s export <file_name.pgm> <threshold>\n", programName);
  printf("\t-%s sort-index\n", programName);
  printf("\t-%s list\n", programName);
}

int main(int argc, char *argv[]){

  _mkdir("index_parts");
  _mkdir("index_backup");

  if(argc < 2){
    printExplanation("./program");
  }

  // ./main import <image.pgm>
  if(strcmp(argv[1], "import") == 0){
    importImage(argv[2]);
    return 0;
  }
  else if(strcmp(argv[1], "export") == 0){
    char outputName[200];
    if(argc < 3){
      printf("export command accepts:\n\t-%s export <file_name.pgm>\n\t-%s export <file_name.pgm> reverse\n\t-%s export <file_name.pgm> threshold\n", "./program", "./program", "./program");
    }
    // ./main export <image.pgm>
    else if(argc < 4){
      sprintf(outputName, "output_%s", argv[2]);
      exportImage(argv[2], outputName, 0, 0);
    }
    // ./main export <image.pgm> reverse
    else if(strcmp(argv[3], "reverse") == 0){
      sprintf(outputName, "reverse_%s", argv[2]);
      exportImage(argv[2], outputName, 2, 0);
    }
    // ./main export <image.pgm> <threshold>
    else if(argc == 4){
      int threshold = atoi(argv[3]);
      sprintf(outputName, "threshold_%d_%s", threshold, argv[2]);
      exportImage(argv[2], outputName, 1, threshold);
    }
    else{
      printf("export command accepts:\n\t-%s export <file_name.pgm>\n\t-%s export <file_name.pgm> reverse\n\t-%s export <file_name.pgm> threshold\n", argv[0], argv[0], argv[0]);
    }
  }
  // ./main list
  else if(strcmp(argv[1], "list") == 0){
    printIndexFile();
  }
  // ./main sort-index
  else if(strcmp(argv[1], "sort-index") == 0){
    FILE* index = fopen(INDEX_FILE, "rb");
    sortIndex(index);
  }
  else{
    printExplanation("./program");
  }

  return 0;
}