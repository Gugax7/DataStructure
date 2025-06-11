#include <stdio.h>
#include <stdlib.h>
#define MAX 100
// okay so first thing i need to understand the filo logic of a pile
// so lets do the pile:

typedef struct{
    int top;
    int elements[MAX];
}Pile;

void initializePile(Pile* pile){
    pile->top = -1;
}

int static_pop(Pile* pile){
    if(pile->top == -1){
        printf("underflow\n");
        return -1;
    }
    return pile->elements[pile->top--];
}

void static_push(Pile* pile, int i){
    if(pile->top == MAX - 1){
        printf("overflow!\n");
        return;
    }
    pile->elements[++pile->top] = i;
    
}

int main(){

    return 0;
}