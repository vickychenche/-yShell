#include "variables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "io_helpers.h"

Node *create_node(char *k, char *v, Node *next){

  Node *new_node = malloc(sizeof(Node));
  new_node->key = malloc(strlen(k)+sizeof(char));
  new_node->value = malloc(strlen(v)+sizeof(char));
  strncpy(new_node->key, k, strlen(k)+sizeof(char));
  strncpy(new_node->value, v, strlen(v)+sizeof(char));
  new_node->next = next;
  return new_node;
}

int *edit_node(Node *front, char *val){
  free(front->value);
  front->value = malloc(strlen(val) +sizeof(char));
  strncpy(front->value, val, strlen(val) +sizeof(char));
  return 0;
}

Node *new_node(char **tokens, Node *front){

    char *ptr_one = strtok (*tokens, "=");
    char *ptr_two = strtok(NULL, " ");
    char *key = get_string(ptr_one);
    char *val = get_string(ptr_two);
    int exist = check_key_exist(key, val, front);
    if (exist == 1){
      return front;
    }
    else{
      return create_node(get_string(ptr_one),  get_string(ptr_two), front);
    }




}

int check_key_exist(char *key ,char *val, Node *front){
  while (front != NULL){
    if(strncmp(key, front->key, strlen(key)) == 0){
      edit_node(front, val);
      return 1;
    }
    front = front->next;
  }
  return 0;
}

char *get_string(char *curr_ptr){
  char *token_arr[MAX_STR_LEN];
  int i = 0;
  while (*curr_ptr != '\0'){
    token_arr[i] = curr_ptr;
    i ++;
    curr_ptr ++;
  }
  token_arr[i] = NULL;
  return *token_arr;
}
