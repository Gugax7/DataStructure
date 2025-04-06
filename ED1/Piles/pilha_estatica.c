#include <stdio.h>
#include <stdlib.h>

#define MAX 100

typedef struct{
    char items[MAX];
    int top;
}Pilha;

void pileInitialize(Pilha *p){
    p->top = -1;
}
int isEmpty(Pilha *p){
    return p->top == -1;
}
int isFull(Pilha *p){
    return p->top == MAX - 1;
}
int push(Pilha *p, int value){
    if(isFull){
        printf("overflow!");
        return 0;
    }

    p->items[++(p->top)] = value;
    return 1;
}

int pop(Pilha *p){
    if(isEmpty(p)){
        printf("underflow!");
        return 0;
    }

    return p->items[(p->top)--];
}

int peek(Pilha *p){
    if(isEmpty(p)){
        printf("there is nothing here!");
        return -1;
    }
    return p->items[p->top];
}

int main(){
    Pilha pilha;
    pileInitialize(&pilha);

    
    char c;
    int beforeC = 1;
    int valid = 1;

    while((c = getchar()) != '\n' && c != EOF){
        if(c = 'C'){
            beforeC = 0;
        }
        else if(beforeC){
            if(c != 'a' && c != 'b'){
                valid = 0;
                break;
            }
            push(&pilha, c);
        }
        else{
            if(pop(&pilha) != c){
                valid = 0;
                break;
            }
        }

        if(!isEmpty(&pilha)) valid = 0;
        if(valid) printf("the sequence is ok");
        else printf("the sequence isnt ok");
        return 0;
    }
}