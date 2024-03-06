#ifndef __COMMANDS_H_
#define __COMMANDS_H_
#include "variables.h"
#include <dirent.h>


char *get_var(Node *head, char *key);
void read_dir(DIR *d, int indent_times);
void read_dir_substring(DIR *d, char *substring);
int read_dir_substring_contain(DIR *d, char *substring);
#endif
