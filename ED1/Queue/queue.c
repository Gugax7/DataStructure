#include <stdlib.h>
#include <stdio.h>
#define MAX 100

typedef struct {
    int front, rear;
    int elements[MAX];
} Line;

void initializeLine(Line *L) {
    L->front = 0;
    L->rear = -1;
}

int isEmpty(Line *L) {
    return L->rear == -1;
}

int insert(Line *L, int element) {
  // Check for overflow
  if (L->rear != -1 && (L->rear + 1) % MAX == L->front) { 
      printf("OVERFLOW! Element not added!\n");
      return 0;
  }
  // Increment rear and insert the element
  L->rear = (L->rear + 1) % MAX; 
  L->elements[L->rear] = element;
  printf("front = %d\nrear = %d\n", L->front, L->rear);
  return 1; // Indicate success
}

int removeElement(Line *pnt_q) {
    if (isEmpty(pnt_q)) { // Test if the queue is empty
        printf("UNDERFLOW! No element to remove.\n");
        exit(1); // Exit the program or handle the error appropriately
    }

    int removedElement = pnt_q->elements[pnt_q->front]; // Get the element to remove

    if (pnt_q->front == pnt_q->rear) { // Queue becomes empty
        pnt_q->front = 0;
        pnt_q->rear = -1;
    } else {
        pnt_q->front = (pnt_q->front + 1) % MAX; // Circular increment
    }
    printf("front = %d\nrear = %d\n", pnt_q->front, pnt_q->rear);
    return removedElement; // Return the removed element
}

int main() {
    Line myLine;
    initializeLine(&myLine);

    insert(&myLine, 10);
    insert(&myLine, 20);
    insert(&myLine, 30);

    printf("Removed: %d\n", removeElement(&myLine));
    printf("Removed: %d\n", removeElement(&myLine));

    insert(&myLine, 40);
    printf("Removed: %d\n", removeElement(&myLine));
    printf("Removed: %d\n", removeElement(&myLine));

    return 0;
}