#ifndef __VARIABLES_H__
#define __VARIABLES_H__

typedef struct node {
  char *key;
  char *value;
  struct node *next;
}Node;

Node *new_node(char **tokens, Node *front);

Node *create_node(char *key, char *word, Node *next);

int *edit_node(Node *front, char *val);
char **m2_input(char *in_ptr, char **values);

int check_key_exist(char *key ,char *var, Node *front);

char *get_string(char *curr_ptr);

#endif
