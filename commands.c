#include "commands.h"
#include "io_helpers.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>

/* Take a key and return variable
*/
char *get_var(Node *head, char *key){
  Node *temp = head;
  if (strlen(key) == 1){
    return "$";
  }
  key = key + sizeof(char);
  while(temp != NULL){
    if (strncmp(temp->key, key, strlen(key)) == 0){
      return(temp->value);
    }
    temp = temp->next;
  }
  return key;
}

/* read through the given directery and dispaly the name
*/
void read_dir(DIR *d, int indent_times){
  struct dirent *dir;
  while ((dir = readdir(d)) != NULL) {
    for (size_t i = 0; i < indent_times; i++) {
      display_message("\t");
    }
    display_message(dir->d_name);

    display_message("\n");
    //fflush(stdout);

  }
}

void read_dir_substring(DIR *d, char *substring){
  struct dirent *dir;
  while ((dir = readdir(d)) != NULL) {
    if (strstr(dir->d_name, substring)){
      display_message(dir->d_name);
      display_message("\n");
    }
  }
}

int read_dir_substring_contain(DIR *d, char *substring){
  struct dirent *dir;
  while ((dir = readdir(d)) != NULL) {
    if (strstr(dir->d_name, substring)){
      return 1;
    }
  }
  return 0;
}