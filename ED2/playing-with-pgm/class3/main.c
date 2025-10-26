#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
  int* values;
  int width, height;
  int grayMax;
  char config[3];
}PgmImage;

typedef struct{
  char *fileName[256];
  long offset;
  long size;
}Index;

void convertImageToStruct(FILE *pgm, PgmImage *object){
  
  // type
  char config[3];
  int width, height;
  int maxGray;
  

  fscanf(pgm,"%s", config);

  if(config[0] != 'P' || config[1] != '2'){
    printf("Only P2 formats accepted in the moment\n");
    return;
  }

  strcpy(object->config, config);

  // jump comments
  // size x size

  fscanf(pgm, "%d %d", &width, &height);
  fscanf(pgm, "%d", &maxGray);

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

int importImage(char * pgmFile, char * databaseFile, char* indexFile){

  FILE* pgmStream;
  FILE* databaseStream;
  FILE* indexStream;

  pgmStream = fopen(pgmFile, "r");
  databaseStream = fopen(databaseFile, "ab+");

  if(!pgmStream || !databaseStream) return 500;

  PgmImage imageObject;
  convertImageToStruct(pgmStream, &imageObject);

  fseek(databaseStream, 0, SEEK_END);

  long currentOffSet = ftell(databaseStream);

  fwrite(imageObject.config, sizeof(char), 3, databaseStream);
  fwrite(&imageObject.width, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.height, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.grayMax, sizeof(int), 1, databaseStream);
  fwrite(imageObject.values, sizeof(int), imageObject.height * imageObject.width, databaseStream);

  indexStream = fopen(indexFile, "a");

  fseek(databaseStream, 0, SEEK_END);

  const size = ftell(databaseStream) - currentOffSet;

  fprintf(indexFile, "%s %ld %ld",
    pgmFile,
    currentOffSet,
    size);

  fclose(pgmStream);
  fclose(databaseStream);

  free(imageObject.values);
  
  return 0;
}

int main(){
  
  importImage("barbara.pgm", "output.bin", "index.txt");

  return 0;
}