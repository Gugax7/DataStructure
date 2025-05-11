#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
  int data;
  struct Node* next1;
  struct Node* next2;
  struct Node* next3;
}Node;

Node* createNode(int data){
  Node* newNode = (Node*)malloc(sizeof(Node));
  if(!newNode){
    printf("memory allocation failed!");
    return NULL;
  }
  newNode->data = data;
  newNode->next1 = NULL;
  newNode->next2 = NULL;
  newNode->next3 = NULL;
  return newNode;
}
Node* getNode(Node* father, int data) {
  if (!father) {
    return NULL;
  }
  if (father->data == data) {
    return father;
  }

  // Search in the first child
  Node* found = getNode(father->next1, data);
  if (found) return found;

  // Search in the second child
  found = getNode(father->next2, data);
  if (found) return found;

  // Search in the third child
  return getNode(father->next3, data);
}



int main(){

    Node* root = createNode(1);

    // Add children to the root
    root->next1 = createNode(2);
    root->next2 = createNode(3);
    root->next3 = createNode(4);

    // Add children to one of the root's children
    root->next1->next1 = createNode(5);
    root->next1->next2 = createNode(6);

    // Add children to another child
    root->next2->next1 = createNode(7);


    Node* myNode = getNode(root, 7);
    root->next2->next1->next1 = createNode(10);
    printf("i expect value 10 here: %d\n", myNode->next1->data);


  return 0;
}