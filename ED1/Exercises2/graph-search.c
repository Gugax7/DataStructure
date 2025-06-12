#include <stdio.h>
#include <stdlib.h>

#define MAX 100



void breadth_search(int graph[MAX][MAX], int amount_of_nodes){

  // create a queue to organize my nodes

  int queue[MAX], front = 0, rear = 0;
  int already_tested[MAX] = {0};

  queue[front] = 0;
  already_tested[0] = 1; 
  int current_node = queue[front];
  front++;
  while(front > rear){
    for(int j = 0; j < amount_of_nodes; j++){
      // so right here i am on the node, searching for who he is connecting
      if(graph[current_node][j] == 1 && already_tested[j] == 0){
        // and here im gonna put it into the queue
        // i need to think a way to ignore the already tested nodes...
        queue[front] = j;
        already_tested[j] = 1;
        front++;
      }
    }
    printf("Okay i found this node: %d\n", queue[rear]);
    rear++;
    current_node = queue[rear];
  }

}

int main(){
  
  int graph[100][100] = {
    {0,1,1,0,0,0},
    {1,0,0,1,0,0},
    {1,0,0,0,0,1},
    {0,1,0,0,1,0},
    {0,0,0,1,0,1},
    {0,0,1,0,1,0}
  };

  breadth_search(graph,6);

  return 0;
}