#include <stdio.h>
#include <stdlib.h>
#include <string.h>

  
  int graph[10][10] = {
    {1,1,0,0,1,0},
    {0,0,0,0,0,1},
    {0,1,0,0,1,0},
    {0,0,0,0,1,1},
    {0,0,0,0,0,1},
    {1,0,1,0,0,0}
  };

  int num_nodes = 6;

  char v_zero[10][10][10] = {
    {"c","a","^","^","b","^"},
    {"^","^","^","^","^","g"},
    {"^","h","^","i","^","^"},
    {"^","^","^","^","e","d"},
    {"^","^","^","^","^","f"},
    {"k","^","j","^","^","^"}
  };

  char v_one[10][10][10] = {
    {"^","a","^","^","b","^"},
    {"^","^","^","^","^","g"},
    {"^","h","^","i","^","^"},
    {"^","^","^","^","e","d"},
    {"^","^","^","^","^","f"},
    {"k","^","j","^","^","^"}
  };

  // now i need to create a result matrix, that is a string matrix...
  // for sure i dont know how to do this right here... but we can try

  char result1[10][10][10] = {""};
  char result2[10][10][10] = {""};
  char hat[2] = "^";

  char current_d[100] = {'c','^','^','^','^','^'};

  // okay now comes the interesting part
  // first of all, we need to understand how to multiply matrixes:

  int mat1[100][100] = {
    {1,2,3},
    {4,5,6},
    {7,8,9}
  };

  int mat2[100][100] = {
    {1,9,3},
    {2,6,3},
    {7,8,1}
  };

  // for each line column we will have to sum the product of each element in column and line.

int main(){

  for(int i = 0; i < num_nodes; i++){
    for(int j = 0; j < num_nodes; j++){
      strcpy(result1[i][j], "");

      int first_path_segment_for_cell = 1;

      for(int k = 0; k < num_nodes; k++){
        // i need to do this operation but with strings now: result[i][j]+= mat1[i][k] * mat2[k][j];
        if(v_one[i][k][0] != '^' && v_zero[k][j][0] != '^'){
          if(!first_path_segment_for_cell){
            strcat(result1[i][j], "^");
          }

          else{
            first_path_segment_for_cell = 0;
          }

          strcat(result1[i][j],v_one[i][k]);
          strcat(result1[i][j],v_zero[k][j]);
        }
      }

      if(strlen(result1[i][j]) == 0){
        strcat(result1[i][j], "^");
      }
    }
  }



  // TODO: change both matrixes and calculate v1 * v0 
  //       that is probably just changing this last cat

    
  for(int i = 0; i < num_nodes; i++){
    for(int j = 0; j < num_nodes; j++){
      strcpy(result2[i][j], "");

      int first_path_segment_for_cell = 1;

      for(int k = 0; k < num_nodes; k++){
        // i need to do this operation but with strings now: result[i][j]+= mat1[i][k] * mat2[k][j];
        if(v_one[k][j][0] != '^' && v_zero[i][k][0] != '^'){
          if(!first_path_segment_for_cell){
            strcat(result2[i][j], "^");
          }

          else{
            first_path_segment_for_cell = 0;
          }

          strcat(result2[i][j],v_zero[i][k]);
          strcat(result2[i][j],v_one[k][j]);
        }
      }
      if(strlen(result2[i][j]) == 0){
        strcat(result2[i][j], "^");
      }
    }
  }

  // TODO: create a new for checking if has intersections
  //       and using this intersections to create v2 and d2
  char v_two[10][10][10] = {""};
  for(int i = 0; i < num_nodes; i++){
    for(int j = 0; j < num_nodes; j++){
      if(strcmp(result1[i][j],result2[i][j]) == 0){
        strcpy(v_two[i][j], result1[i][j]);
      }
      else{
        strcpy(v_two[i][j], "^");
      }
    }
  }

  // TODO: create a form to calculate it all ad infinitum

  // int result[100][100] = {0};
  // for(int i = 0; i < 3; i++){
  //   for(int j = 0; j < 3; j++){
  //     for(int k = 0; k < 3; k++){
  //       result[i][j]+= mat1[i][k] * mat2[k][j]; 
  //     }
  //   }
  // }

  for(int g = 0; g < 6; g++){
    for(int f = 0; f < 6; f++){
    printf("%s ", result1[g][f]);
    }
    printf("\n");
  }

printf("\n");
printf("\n");
printf("\n");

  for(int g = 0; g < 6; g++){
    for(int f = 0; f < 6; f++){
    printf("%s ", result2[g][f]);
    }
    printf("\n");
  }

printf("\n");
printf("\n");
printf("\n");

  for(int g = 0; g < 6; g++){
    for(int f = 0; f < 6; f++){
    printf("%s ", v_two[g][f]);
    }
    printf("\n");
  }
}