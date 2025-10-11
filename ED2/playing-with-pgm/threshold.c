#include <stdio.h>
#include <stdlib.h>

void threshold_pgm(const char *input, const char *output, int threshold){
  FILE *fin = fopen(input, "r");
  FILE *fout = fopen (output, "w");

  if(!fin || !fout){
    printf("Error opening files.\n");
    return;
  }

  char config[3];
  int width, height, maxval;

  fscanf(fin, "%2s", config);

  if(config[0] != 'P' || config[1] != '2'){
    printf("Only P2 (ASCII) PGM supported. \n");

    fclose(fin);
    fclose(fout);
  }

  // skip comments

  fscanf(fin, "%d %d", &width, &height);
  fscanf(fin, "%d", &maxval);

  fprintf(fout, "P2\n%d %d\n255\n", width, height);

  for(int i = 0; i < width * height; i++){
    int pixel;

    fscanf(fin, "%d", &pixel);

    if(pixel > threshold) pixel = 255;
    else pixel = 0;

    fprintf(fout, "%d ", pixel);
  }

  fclose(fin);
  fclose(fout);

}

int main() {
    int threshold = 100;
    threshold_pgm("barbara.pgm", "output.pgm", threshold);
    printf("Done!\n");
    return 0;
}