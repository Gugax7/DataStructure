#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
    int info;
    struct Node* next;
}Node;

#include <stdio.h>
#include <stdlib.h>

// Estrutura do nó
typedef struct Node {
    int info;
    struct Node* next;
} Node;

// Adiciona um nó no início da lista
void addNodeAtBeginning(Node** head, int info) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Erro ao alocar memória\n");
        exit(1);
    }
    new_node->info = info;
    new_node->next = *head;
    *head = new_node;
}

// Adiciona um nó no final da lista
void addNodeAtEnd(Node** head, int info) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Erro ao alocar memória\n");
        exit(1);
    }
    new_node->info = info;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
}

// Adiciona um nó em uma posição específica
void addNodeAtMiddle(Node** head, int info, int position) {
    if (position <= 1) {
        addNodeAtBeginning(head, info);
        return;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Erro ao alocar memória\n");
        exit(1);
    }
    new_node->info = info;

    Node* temp = *head;
    int count = 1;

    while (temp != NULL && count < position - 1) {
        temp = temp->next;
        count++;
    }

    if (temp == NULL) {
        printf("Posição inválida\n");
        free(new_node);
        return;
    }

    new_node->next = temp->next;
    temp->next = new_node;
}

// Função para imprimir a lista
void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%d -> ", temp->info);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Função principal
int main() {
    Node* head = NULL;

    // Testando as funções
    addNodeAtBeginning(&head, 10);
    addNodeAtEnd(&head, 20);
    addNodeAtMiddle(&head, 15, 2);
    addNodeAtEnd(&head, 30);
    addNodeAtMiddle(&head, 5, 1);

    printf("Lista encadeada: ");
    printList(head);

    return 0;
}

int main(){

    return 0;
}