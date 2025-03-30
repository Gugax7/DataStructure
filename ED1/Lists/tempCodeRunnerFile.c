#include <stdlib.h>
#include <stdio.h>
#define MAX 100

typedef struct{
    int info, next;
}Node;

int disp = 0;
Node list[MAX];

Node* manual_alloc(){
    if(disp == -1){
        printf("there is no memory left\n");
        return NULL;
    }
    int last_disp = disp;
    disp = list[disp].next;
    return &list[last_disp];

}

void free(Node *node){
    node->next = disp;
    disp = node - list;
}
void add_node_at_end(Node *L,int info){
    while(L->next != -1){
        L = &list[L->next];
    }
    L->next = disp;
    Node *new_node = manual_alloc();
    if(new_node == NULL){
        printf("No memory available\n");
        return;
    }
    new_node->info = info;
    new_node->next = -1;
}
int main(){
    int i = 0;
    for(i = 0; i < MAX - 1; i++){
        list[i].next = i+1;
    }
    list[MAX - 1].next = -1;

    Node *head = &list[disp];
    add_node_at_end(head, 10);
    add_node_at_end(head, 15);
    add_node_at_end(head, 12);
    add_node_at_end(head, 19);
    add_node_at_end(head, 30);
    add_node_at_end(head, 20);
    add_node_at_end(head, 13);
    add_node_at_end(head, 18);
    add_node_at_end(head, 16);
    add_node_at_end(head, 22);
    add_node_at_end(head, 9);

    // now i will chose values between 15 and 20 to maintain on list

    
    Node *temp = head;
    Node *prev = NULL;
    do{
        if(temp->info <= 15 || temp->info >= 20){
            if(temp == head && temp->next != -1){
                head = &list[temp->next];
                free(temp);
                temp = head;
                continue;
            }
            else{
                prev->next = temp->next;
                free(temp);
                temp = &list[prev->next];
                continue;
            }
        }
        prev = temp;
        temp = &list[temp->next];
    }while(temp->next != -1);
    printf("\nNode: %i next: %i", temp->info,temp->next);
    printf("\n");
    return 0;

}