#include <stdio.h>
#include <stdlib.h>

void threshold_pgm(const char *input, const char *output, int threshold){
  FILE *fin = fopen(input, "r");
  FILE *fout = fopen (output, "w");

  if(!fin || !fout){
    printf("Error opening files.\n");
    return;
  }

  char magic[3];
  int width, height, maxval;

  fscanf(fin, "%2s", magic);

  if(magic[0] != 'P' || magic[1] != '2'){
    printf("Only P2 (ASCII) PGM supported. \n");

    fclose(fin);
    fclose(fout);
  }

  // skip comments, probably when we send we might delete it, i dont know how useful it is actually.

  int c;
  while((c = fgetc(fin)) == '#'){
    while(fgetc(fin) != '\n');
  }

  ungetc(c, fin);

  fscanf(fin, "%d %d", &width, &height);
  fscanf(fin, "%d", &maxval);

  fprintf(fout, "P2\n%d %d\n255\n", width, height);

  for(int i = 0; i < width * height; i++){
    int pixel;

    fscanf(fin, "%d", &pixel);

    pixel = maxval - pixel;
    
    fprintf(fout, "%d ", pixel);
  }

  fclose(fin);
  fclose(fout);

}

int main() {
    threshold_pgm("barbara.pgm", "output.pgm", 100);
    printf("Done!\n");
    return 0;
}