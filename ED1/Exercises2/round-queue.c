#include <stdio.h>
#include <stdlib.h>
#define MAX 10

struct queue{
  int front, rear;
  int *list;
};

void initialize(struct queue *queue){
  queue->front = 0;
  queue->rear = 0;
  queue->list = malloc(MAX * sizeof(int));
}

int is_full(struct queue* queue){
  return (queue->front + 1) % MAX == queue->rear;
}

int is_empty(struct queue* queue){
  return queue->front == queue->rear;
}

void push(struct queue *queue, int value){
  if(is_full(queue)){
    printf("Overflow! Queue is full!\n");
    return;
  }

  queue->list[queue->front] = value;
  queue->front = (queue->front + 1) % MAX;
}

void pop(struct queue *queue){
  if(is_empty(queue)){
    printf("Underflow! The queue is empty!\n");
    return;
  }

  queue->rear = (queue->rear + 1) % MAX; 
}

void print_queue(struct queue* queue){
  if(is_empty(queue)){
    return;
  }

    int i = queue->rear;

    while(i != queue->front){
      printf("%d, ", queue->list[i]);
      i = (i + 1) % MAX;
    }
}

int main(){
  
  struct queue* queue;
  initialize(queue);

  push(queue, 10);
  push(queue, 10);
  push(queue, 2);
  push(queue, 40);
  push(queue, 16);
  push(queue, 17);
  push(queue, 102);
  push(queue, 134);
  push(queue, 62);
  push(queue, 62);

  print_queue(queue);

  return 0;
}