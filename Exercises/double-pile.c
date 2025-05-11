#include <stdio.h>
#include <stdlib.h>
#define MAX 100

typedef struct {
    int elements[MAX];
    int top1; // Top for the first pile
    int top2; // Top for the second pile
} DoublePile;

// Initialize the double pile
void initializeDoublePile(DoublePile* pile) {
    pile->top1 = -1;          // First pile starts from the beginning
    pile->top2 = MAX;         // Second pile starts from the end
}

// Push an element onto the first pile
void push1(DoublePile* pile, int value) {
    if (pile->top1 + 1 == pile->top2) {
        printf("Overflow! No space for pile 1.\n");
        return;
    }
    pile->elements[++pile->top1] = value;
}

// Push an element onto the second pile
void push2(DoublePile* pile, int value) {
    if (pile->top2 - 1 == pile->top1) {
        printf("Overflow! No space for pile 2.\n");
        return;
    }
    pile->elements[--pile->top2] = value;
}

// Pop an element from the first pile
int pop1(DoublePile* pile) {
    if (pile->top1 == -1) {
        printf("Underflow! Pile 1 is empty.\n");
        return -1;
    }
    return pile->elements[pile->top1--];
}

// Pop an element from the second pile
int pop2(DoublePile* pile) {
    if (pile->top2 == MAX) {
        printf("Underflow! Pile 2 is empty.\n");
        return -1;
    }
    return pile->elements[pile->top2++];
}

// Test the double pile
int main() {
    DoublePile pile;
    initializeDoublePile(&pile);

    // Test pile 1
    push1(&pile, 10);
    push1(&pile, 20);
    printf("Popped from pile 1: %d\n", pop1(&pile)); // Should print 20

    // Test pile 2
    push2(&pile, 30);
    push2(&pile, 40);
    printf("Popped from pile 2: %d\n", pop2(&pile)); // Should print 40

    // Test overflow
    for (int i = 0; i < MAX / 2; i++) {
        push1(&pile, i);
        push2(&pile, i);
    }

    return 0;
}