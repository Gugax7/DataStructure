#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_FILE "index.txt"
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
  

  fscanf(pgm,"%s", config);

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

  indexStream = fopen(INDEX_FILE, "a");

  fseek(databaseStream, 0, SEEK_END);

  long size = ftell(databaseStream) - currentOffSet;

  fprintf(indexStream, "%s %ld %ld\n",
    pgmFile,
    currentOffSet,
    size);

  fclose(pgmStream);
  fclose(databaseStream);
  fclose(indexStream);

  free(imageObject.values);
  
  return 0;
}

int exportImage(char * imageName, char *outputName){
  FILE* outputStream = fopen(outputName, "w");
  FILE* indexStream = fopen(INDEX_FILE, "r");

  char inFileName[256];
  long offset;
  long size;
  int position = 0;


  // Find the index with this outputName;
  do{
    position++;
    if(fscanf(indexStream, "%s %ld %ld",inFileName, &offset, &size) != 3){
      return 1;
    };
  }while(strcmp(inFileName, imageName) != 0);

  //printf("on position %d: found!, %s %ld %ld", position, inFileName, offset, size);

  // from here i actually have this images offsets, sizes and positions

  FILE* databaseStream = fopen(DATABASE_FILE, "rb");

  PgmImage outputObject;

  fseek(databaseStream, offset, SEEK_SET);

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

  fprintf(outputStream, "%s\n", outputObject.config);
  fprintf(outputStream, "%d %d\n", outputObject.width, outputObject.height);
  fprintf(outputStream, "%d\n", outputObject.grayMax);

  for(int i = 0; i < outputObject.width * outputObject.height; i++){
    fprintf(outputStream, "%d ", outputObject.values[i]);
  }
  fclose(outputStream);
  fclose(indexStream);
  fclose(databaseStream);

  return 0;
}

int main(){
  
  // importImage("barbara.pgm");
  // importImage("seila.pgm");
  // importImage("limiar.pgm");

  exportImage("seila.pgm","dk.pgm");

  return 0;
}