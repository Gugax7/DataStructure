#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_FILE "index.txt"
#define DATA_FILE "imagens.bin"
#define TEMP_DATA_FILE "imagens_temp.bin"
#define TEMP_INDEX_FILE "index_temp.txt"


typedef struct {
    char nome[256];
    int limiar;
    long offset;
    long tamanho;
    int removido; // 0 ou 1
} IndexEntry;


typedef struct {
    int h, w, cinza;
    short int* pixels; // valores 0..cinza
} imgb;


imgb read_pgm(FILE* fimg) {
  imgb I = {0, 0, 0, NULL};
  char tipo[3];
  int h, w, cinza;

  if (fscanf(fimg, "%2s", tipo) != 1) return I;
  if (strcmp(tipo, "P5") != 0 && strcmp(tipo, "P2") != 0) return I;
  fgetc(fimg);

  char ch = fgetc(fimg);
  while (ch == '#') {
    while (fgetc(fimg) != '\n');
    ch = fgetc(fimg);
  }
  ungetc(ch, fimg);

  if (fscanf(fimg, "%d%d", &w, &h) != 2) return I;
  if (fscanf(fimg, "%d", &cinza) != 1) return I;
  fgetc(fimg);

  I.w = w;
  I.h = h;
  I.cinza = cinza;
  I.pixels = (short int*)malloc(w * h * sizeof(short int));
  if(I.pixels == NULL) return I;

  if (strcmp(tipo, "P5") == 0) {
    for (int i = 0; i < w * h; i++) {
      unsigned char byte;
      if (fread(&byte, sizeof(unsigned char), 1, fimg) != 1) {
        free(I.pixels);
        I.pixels = NULL;
        return I;
      }
      I.pixels[i] = (short int)byte;
    }
  } else {
      for (int i = 0; i < w * h; i++) {
        if (fscanf(fimg, "%hd", &I.pixels[i]) != 1) {
          free(I.pixels);
          I.pixels = NULL;
          return I;
        }
      }
  }
  return I;
}


void write_pgm (const char *filename, imgb *I) {
  FILE* fpgm = fopen(filename, "wb");
  if(fpgm == NULL) {
    perror("Erro ao abrir arquivo para escrita PGM");
    return;
  }

  fprintf(fpgm, "P2\n");
  fprintf(fpgm, "%d %d\n", I->w, I->h);
  fprintf(fpgm, "%d\n", I->cinza);

  for (int i=0; i < I->w * I->h; i++) {
    fprintf(fpgm, "%d ", I->pixels[i]);
  }

  fclose(fpgm);
}


void limiarizar(imgb *I, int lim) {
  for(int i=0; i < I->w*I->h; i++) {
    if (I->pixels[i] < lim) I->pixels[i] = 0;
    else I->pixels[i] = I->cinza;
  }
}