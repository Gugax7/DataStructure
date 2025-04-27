#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int vertex;
    int weight; // I will use only in case that Graph is weighted
    struct Node* next;

} Node;

typedef struct{

    int num_vertices;
    int is_directed;
    int is_weighted; //i am using these ints just to have the information like boolean, yes for some reason c dont have boolean.
    Node** adj_list;

}Graph;
