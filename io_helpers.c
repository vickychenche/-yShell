#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <stdbool.h>

#include "io_helpers.h"


// ===== Output helpers =====

/* Prereq: str is a NULL terminated string
 */
void display_message(char *str) {
    write(STDOUT_FILENO, str, strnlen(str, MAX_STR_LEN));
}


/* Prereq: pre_str, str are NULL terminated string
 */
void display_error(char *pre_str, char *str) {
    write(STDERR_FILENO, pre_str, strnlen(pre_str, MAX_STR_LEN));
    write(STDERR_FILENO, str, strnlen(str, MAX_STR_LEN));
    write(STDERR_FILENO, "\n", 1);
}


// ===== Input tokenizing =====

/* Prereq: in_ptr points to a character buffer of size > MAX_STR_LEN
 * Return: number of bytes read
 */
ssize_t get_input(char *in_ptr) {
    int retval = read(STDIN_FILENO, in_ptr, MAX_STR_LEN+1); // Not a sanitizer issue since in_ptr is allocated as MAX_STR_LEN+1
    int read_len = retval;
    if (retval == -1) {
        read_len = 0;
    }
    if (read_len > MAX_STR_LEN) {
        read_len = 0;
        retval = -1;
        write(STDERR_FILENO, "ERROR: input line too long\n", strlen("ERROR: input line too long\n"));
        int junk = 0;
        while((junk = getchar()) != EOF && junk != '\n');
    }
    in_ptr[read_len] = '\0';
    return retval;
}

/* Prereq: in_ptr is a string, tokens is of size >= len(in_ptr)
 * Warning: in_ptr is modified
 * Return: number of tokens.
 */
size_t tokenize_input(char *in_ptr, char **tokens) {
    // TODO, uncomment the next line.
    char *curr_ptr = strtok (in_ptr, DELIMITERS);
    size_t token_count = 0;
    char *needle = "|";
    char *empty = "";
    char * rest = NULL;
    char * before_rest;
    
    
    while (curr_ptr != NULL) {  // TODO: Fix this
        // TODO: Fix this
        //tokenize_helper(curr_ptr, &tokens, &token_count);
        if(strncmp(curr_ptr,needle,MAX_STR_LEN) == 0){
            tokens[token_count] = curr_ptr;
            token_count += 1;
        }else{
            char *f = strstr(curr_ptr, needle);
            if(f == NULL){
                tokens[token_count] = curr_ptr;
                token_count += 1;
            }
            else{
                f = strtok_r(curr_ptr, needle, &rest);
                while (f != NULL){
                tokens[token_count] = f;
                token_count += 1;
                tokens[token_count] = "|";
                token_count += 1;
                before_rest = rest;
                if (strstr(rest, needle) == NULL){
                    break;
                }
                f = strtok_r(rest, needle, &rest);
                
                }
                if(strncmp(before_rest, empty, MAX_STR_LEN) != 0){
                    tokens[token_count] = before_rest;
                    token_count += 1;
                }
                
            }
        }
        
        curr_ptr = strtok(NULL, DELIMITERS);
        //token_count += 1;
    }
    tokens[token_count] = NULL;
    return token_count;
}

char ** tokenize_helper(char * curr_ptr, char ***tokens, size_t *i){
    char *needle = "|";
    char * rest = NULL;
    char * before_rest;
    char *f = strstr(curr_ptr, needle);
    if(f == NULL){
        (*tokens)[*i] = curr_ptr;
        *i += 1;
    }
    else{
        f = strtok_r(curr_ptr, needle, &rest);
        while (f != NULL){
        (*tokens)[*i] = f;
        *i += 1;
        (*tokens)[*i] = "|";
        *i += 1;
        before_rest = rest;
        f = strtok_r(rest, needle, &rest);
        
        }
        (*tokens)[*i] = before_rest;
        *i += 1;
    }
    
    return *tokens;
}



CNode *create_child(int pid, char **command, CNode *next, int child_num){
    char prompt[MAX_STR_LEN] = "mysh$ ";
    char space[MAX_STR_LEN] = " ";
    char input_buf[MAX_STR_LEN+1];
    CNode *new_node = malloc(sizeof(CNode));
    
    new_node->pid = pid;
    new_node->child_number = child_num;
    int i = 0;
    while(command[i] != NULL){
        strncat(input_buf, command[i], strlen(command[i]));
        strncat(input_buf, space, strlen(space));
        i ++;
    }
    new_node->command = malloc(strlen(input_buf)+sizeof(char));
    strncpy(new_node->command, input_buf, strlen(input_buf)+sizeof(char));
    if(strncmp(input_buf, prompt, MAX_STR_LEN) == 0){
        new_node->active = false;
    }
    else{
        new_node->active = true;
    }
  new_node->next = next;
  return new_node;
}