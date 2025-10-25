#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
  int* values;
  int width, height;
  int grayMax;
  char config[3];
}PgmImage;

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

int importImage(char * pgmFile, char * databaseFile){

  FILE* pgmStream;
  FILE* databaseStream;

  pgmStream = fopen(pgmFile, "r");
  databaseStream = fopen(databaseFile, "wb");

  if(!pgmStream || !databaseStream) return 500;

  PgmImage imageObject;
  convertImageToStruct(pgmStream, &imageObject);

  fwrite(imageObject.config, sizeof(char), 3, databaseStream);
  fwrite(&imageObject.width, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.height, sizeof(int), 1, databaseStream);
  fwrite(&imageObject.grayMax, sizeof(int), 1, databaseStream);
  fwrite(imageObject.values, sizeof(int), imageObject.height * imageObject.width, databaseStream);

  fclose(pgmStream);
  fclose(databaseStream);

  free(imageObject.values);
  
  return 0;
}

int main(){
  
  importImage("barbara.pgm", "output.bin");

  return 0;
}