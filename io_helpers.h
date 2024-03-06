#ifndef __IO_HELPERS_H__
#define __IO_HELPERS_H__

#include <sys/types.h>
#include <stdbool.h>


#define MAX_STR_LEN 64
#define DELIMITERS " \t\n"     // Assumption: all input tokens are whitespace delimited

typedef struct child_node{
    int pid;
    int child_number;
    bool active;
    char *command;
    struct child_node *next;
}CNode;

/* Prereq: pre_str, str are NULL terminated string
 */
void display_message(char *str);
void display_error(char *pre_str, char *str);


/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr);


/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens);
char ** tokenize_helper(char *curr_ptr, char ***tokens, size_t *i);

CNode *create_child(int pid, char **tokens, CNode *next, int child_number);
#endif