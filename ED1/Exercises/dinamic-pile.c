#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
    int value;
    struct Node* next;
}Node;

void dinamic_push(Node** L, int value){
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->next = *L;
    newNode->value = value;
    *L = newNode;
}

int dinamic_pop(Node** L){
    if(*L == NULL){
        printf("underflow\n");
        return -1;
    }
    int keep_value = (*L)->value;
    Node* to_free = *L;
    *L = (*L)->next;
    free(to_free);

    return keep_value;
}

int main(){

    return 0;
}