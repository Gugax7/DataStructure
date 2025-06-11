#include <stdio.h>
#include <stdlib.h>
#define MAX 100

typedef struct{
    int next, info;
}Node;

Node list[MAX];
int dispo = 0;

Node* manualAlloc(){
    if(dispo == -1){
        printf("there is no space left... sorry!");
        return NULL;
    }

    int lastDispo = dispo;
    dispo = list[dispo].next;
    return &list[lastDispo];
}

void manualFree(Node* node){
    // preciso fazer o next dele ser o dispo
    // dispo precisa apontar pro index desse node;
    node->next = dispo;
    dispo = node - list; // ai o dispo vai apontar pro cara que nois livro
}

void addNodeAtEnd(Node* node, int info){

    while(node->next != -1){
        node = &list[node->next];
    }

    node->next = dispo;
    Node* new_node = manualAlloc();
    if(new_node == NULL){
        printf("vish deu pau criando o no");
        return;
    }
    new_node->info = info;
    new_node->next = -1;
}
void addNodeAtBeginning(Node** L, int info) {
    Node* new_node = manualAlloc();
    if (new_node == NULL) {
        printf("vish deu pau criando o no");
        return;
    }
    new_node->next = *L - list;
    new_node->info = info;
    *L = new_node;
}

void addNodeAtMiddle(Node* node, int info, int position) {
    int count = 1;
    Node* prev = NULL;

    // Percorre a lista até encontrar a posição desejada
    while (node != NULL && count < position) {
        prev = node;
        node = &list[node->next];
        count++;
    }

    if (count != position) {
        printf("Posição inválida\n");
        return;
    }
    int node_index = dispo;
    Node* new_node = manualAlloc();
    if (new_node == NULL) {
        printf("vish deu pau criando o no");
        return;
    }
    int temp = prev->next;
    prev->next = node_index;
    new_node->next = temp;
    new_node->info = info;


}

void addNodeAtMiddle(Node* node, int info, int position) {
    int count = 1;
    Node* prev = NULL;

    // Percorre a lista até encontrar a posição desejada
    while (node != NULL && count < position) {
        prev = node;
        node = &list[node->next];
        count++;
    }

    if (count != position) {
        printf("Posição inválida\n");
        return;
    }

    Node* new_node = manualAlloc();
    if (new_node == NULL) {
        printf("vish deu pau criando o no");
        return;
    }

    int new_node_index = new_node - list; // Calcula o índice do novo nó
    new_node->info = info;
    new_node->next = prev->next; // O novo nó aponta para o próximo nó
    prev->next = new_node_index; // O nó anterior aponta para o novo nó
}