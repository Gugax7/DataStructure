#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
  int value;
  struct Node *left, *right;
}Node;

void add_node(Node **head, int value){
  Node *newNode = (Node*)malloc(sizeof(Node));
  newNode->value = value;
  if(*head == NULL){
    *head = newNode;
    (*head)->left = NULL;
    (*head)->right = NULL;
    return;
  }
  if((*head)->value > value) add_node(&((*head)->left), value);
  else if((*head)->value < value) add_node(&((*head)->right), value);
  printf("this value already exists!");
}

int main(){



  return 0;
}