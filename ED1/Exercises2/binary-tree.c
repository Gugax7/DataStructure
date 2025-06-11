#include <stdio.h>
#include <stdlib.h>

struct node{
  int value;
  struct node* left;
  struct node* right;
};

void insert(struct node** root, int value){
  if(*root == NULL){
    *root = (struct node*)malloc(sizeof(struct node));

    if(!(*root)){
      printf("Error creating the root.");
      return;
    }

    (*root)->left = NULL;
    (*root)->right = NULL;
    (*root)->value = value;

    return;
  }

  if(value > (*root)->value){
    insert(&((*root)->right), value);
  }
  else if(value < (*root)->value){
    insert(&((*root)->left), value);
  }
}

// but how can i print it?
// well maybe lets try...

void delete_node(struct node** root, int target){
  if(*root == NULL) return;
  
  if((*root)->value < target){
    delete_node(&((*root)->right), target);
  }

  else if((*root)->value > target){
    delete_node(&((*root)->left), target);
  }
  else{
   if ((*root)->left == NULL && (*root)->right == NULL) {
      // Case 1: No children
      free(*root);
      *root = NULL;

    } 
    else if ((*root)->left == NULL) {
      // Case 2: Only right child
      struct node* temp = *root;
      *root = (*root)->right;

      free(temp);
    }
    else if ((*root)->right == NULL) {
        // Case 2: Only left child
        struct node* temp = *root;
        *root = (*root)->left;

        free(temp);
    }
    else{
      struct node* rep = (*root)->right;
      
      while(rep->left != NULL){
        rep = rep->left;
      }

      (*root)->value = rep->value;
      //delete_node(&((*root)->right), rep->value);
      free(rep);
    }
      
  }
}

void print_tree(struct node* root, int level){
  if(root == NULL) return;

  print_tree(root->right, level + 1);

  for(int i = 0; i < level; i++) printf("   ");

  printf("%d\n", root->value);

  print_tree(root->left, level + 1);
}

struct node* search_node(struct node *root, int target){
  if(root == NULL){
    printf("Sorry i dont found it :C\n");
    return NULL;
  }
  
  if(target > root->value){
    search_node(root->right, target);
  }

  else if(target < root->value){
    search_node(root->left, target);
  }

  else{
    printf("I FOUND IT MASTER!\n");
    return root;
  }

}

int main(){
  struct node* root = NULL;

  insert(&root, 50);
  insert(&root, 76);
  insert(&root, 21);
  insert(&root, 4);
  insert(&root, 32);
  insert(&root, 64);
  insert(&root, 100);
  insert(&root, 83);
  insert(&root, 2);
  insert(&root, 91);
  delete_node(&root, 32);

  print_tree(root, 0);
  search_node(root,91);
  search_node(root,32);
  search_node(root, 7);
  
  

  return 0;
}