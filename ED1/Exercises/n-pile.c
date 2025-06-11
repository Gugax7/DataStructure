#include <stdio.h>
#include <stdlib.h>
#define MAX 100 // Total size of the array
#define N 3     // Number of piles

typedef struct {
    int elements[MAX];
    int tops[N]; // Array to store the top index for each pile
    int size;    // Size of each pile
} NPile;

// Initialize the n-pile
void initializeNPile(NPile* pile) {
    pile->size = MAX / N; // Divide the array into N equal parts
    for (int i = 0; i < N; i++) {
        pile->tops[i] = i * pile->size - 1; // Initialize tops for each pile
    }
}

// Push an element onto a specific pile
void push(NPile* pile, int pileIndex, int value) {
    if (pileIndex < 0 || pileIndex >= N) {
        printf("Invalid pile index!\n");
        return;
    }
    int start = pileIndex * pile->size;
    if (pile->tops[pileIndex] == start + pile->size - 1) {
        printf("Overflow in pile %d!\n", pileIndex);
        return;
    }
    pile->elements[++pile->tops[pileIndex]] = value;
}

// Pop an element from a specific pile
int pop(NPile* pile, int pileIndex) {
    if (pileIndex < 0 || pileIndex >= N) {
        printf("Invalid pile index!\n");
        return -1;
    }
    int start = pileIndex * pile->size;
    if (pile->tops[pileIndex] < start) {
        printf("Underflow in pile %d!\n", pileIndex);
        return -1;
    }
    return pile->elements[pile->tops[pileIndex]--];
}

// Test the n-pile
int main() {
    NPile pile;
    initializeNPile(&pile);

    // Test pile 0
    push(&pile, 0, 10);
    push(&pile, 0, 20);
    printf("Popped from pile 0: %d\n", pop(&pile, 0)); // Should print 20

    // Test pile 1
    push(&pile, 1, 30);
    push(&pile, 1, 40);
    printf("Popped from pile 1: %d\n", pop(&pile, 1)); // Should print 40

    // Test pile 2
    push(&pile, 2, 50);
    push(&pile, 2, 60);
    printf("Popped from pile 2: %d\n", pop(&pile, 2)); // Should print 60

    // Test overflow
    for (int i = 0; i < pile.size + 1; i++) {
        push(&pile, 0, i);
    }

    return 0;
}