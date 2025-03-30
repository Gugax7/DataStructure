#include <stdlib.h>
#include <stdio.h>
#define MAX 100

typedef struct {
    int info, next;
} Node;

int disp = 0;
Node list[MAX];

Node* manual_alloc() {
    if (disp == -1) {
        printf("There is no memory left\n");
        return NULL;
    }
    int last_disp = disp;
    disp = list[disp].next;
    return &list[last_disp];
}

void manual_free(Node *node) {
    node->next = disp;
    disp = node - list;
}

void add_node_at_end(Node *L, int info) {
    while (L->next != -1) {
        L = &list[L->next];
    }
    L->next = disp;
    Node *new_node = manual_alloc();
    if (new_node == NULL) {
        printf("No memory available\n");
        return;
    }
    new_node->info = info;
    new_node->next = -1;
}

void filter_list(Node **head, int min, int max) {
    Node *temp = *head;
    Node *prev = NULL;

    while (temp != NULL && temp->next != -1) {
        if (temp->info < min || temp->info > max) {
            // Remove the node
            if (temp == *head) {
                *head = &list[temp->next];
                manual_free(temp);
                temp = *head;
            } else {
                prev->next = temp->next;
                manual_free(temp);
                temp = &list[prev->next];
            }
        } else {
            prev = temp;
            temp = &list[temp->next];
        }
    }

    // Handle the last node
    if (temp != NULL && (temp->info < min || temp->info > max)) {
        if (temp == *head) {
            *head = NULL;
        } else {
            prev->next = -1;
        }
        manual_free(temp);
    }
}

void print_list(Node *head) {
    Node *temp = head;
    printf("\nList: ");
    while (temp != NULL && temp->next != -1) {
        printf("%i ", temp->info);
        temp = &list[temp->next];
    }
    if (temp != NULL) {
        printf("%i", temp->info);
    }
    printf("\n");
}

int main() {
    int i;
    for (i = 0; i < MAX - 1; i++) {
        list[i].next = i + 1;
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

    // Filter the list to keep values between 15 and 20
    filter_list(&head, 15, 20);

    // Print the filtered list
    print_list(head);

    return 0;
}