#include <stdio.h>
#include <stdlib.h>
#define MAX 2

typedef struct{
  int front, rear;
  int items[MAX];
  int isFull;
}Queue;

void initializeQueue(Queue *q){
  q->front = 0;
  q->rear = -1;
  q->isFull = 0;
}

int isEmpty(Queue *q){
  return q->front - 1 == q->rear && !q->isFull;
}

int insertElement(Queue *q, int element){

  if(q->isFull){
    printf("OVERFLOW!");
    return 0;
  }
  q->rear = (q->rear + 1) % MAX;
  q->items[q->rear] = element;
  if((q->rear + 1) % MAX == q->front && q->rear != -1){
    q->isFull = 1;
    printf("Queue is full!");
  } 
  return 1;
}

int removeElement(Queue *q){

  if(isEmpty(q)){
    printf("Underflow!");
    return 0;
  }

  q->front = (q->front + 1) % MAX;
  q->isFull = 0;
  return 1;
}

int main() {
  Queue myLine;
  initializeQueue(&myLine);

  insertElement(&myLine, 10);
  insertElement(&myLine, 20);
  insertElement(&myLine, 30);

  printf("Removed: %d\n", removeElement(&myLine));
  printf("Removed: %d\n", removeElement(&myLine));

  insertElement(&myLine, 40);
  printf("Removed: %d\n", removeElement(&myLine));
  printf("Removed: %d\n", removeElement(&myLine));

  return 0;
}