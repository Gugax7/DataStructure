#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
    char value;
    struct Node *next;
} Node;

typedef struct{
    Node *top;
} Pile;

void initialize(Pile *p){
    p->top = NULL;
}
int isEmpty(Pile *p){
    return p->top == NULL;
}

int push(Pile *p, char c){
    Node *newNode = (Node*)malloc(sizeof(Node));
    if(!newNode) return 0;
    newNode->value = c;
    newNode->next = p->top;
    p->top = newNode;
    return 1;
}

char pop(Pile *p){
    if(isEmpty(p)){
        return '\0';
    }
    Node *temp = p->top;
    p->top = temp->next;
    char value = temp->value;
    free(temp);
    return value;
}

void freePile(Pile *p){
    while(!isEmpty(p)){
        pop(p);        
    }
}

int main(){

    Pile pile;
    initialize(&pile);

    char c;
    int beforeC = 1;
    int valid = 1;

    printf("Type sequence: \n");
    while((c = getchar()) != '\n' && c != EOF){
        if(c == 'C'){
            beforeC = 0;
        }
        else if(beforeC){
            if(c != 'a' && c != 'b'){
                valid = 0;
                break;
            }
            push(&pile, c);
        }
        else{
            if(pop(&pile) != c){
                valid = 0;
                break;
            }
        }
    }
    if(!isEmpty(&pile)) valid = 0;
    
    if(valid){
        printf("sequence is ok");
    }
    else{
        printf("sequence isnt ok");
    }
}