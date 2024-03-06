#ifndef __BUILTINS_H__
#define __BUILTINS_H__

#include <unistd.h>
#include "variables.h"
#include "io_helpers.h"
#include <stdbool.h>
#include <dirent.h>

#ifndef MAX_BACKLOG
    #define MAX_BACKLOG 5
#endif

#define NOT_PRINTED 22
/* Type for builtin handling functions
 * Input: Array of tokens
 * Return: >=0 on success and -1 on error
 */
typedef ssize_t (*bn_ptr)(char **, Node *, CNode *child);
ssize_t bn_echo(char **tokens, Node *curr, CNode *child);
ssize_t bn_var(char **tokens, Node *curr, CNode *child);
ssize_t bn_ls(char **tokens, Node *curr, CNode *child);
ssize_t bn_cd(char **tokens, Node *curr, CNode *child);
ssize_t bn_cat(char **tokens, Node *curr, CNode *child);
ssize_t bn_wc(char **tokens, Node *curr, CNode *child);
ssize_t bn_pwd(char **tokens, Node *curr, CNode *child);
ssize_t bn_kill(char **tokens, Node *curr, CNode *child);
ssize_t bn_ps(char **tokens, Node *curr, CNode *child);
ssize_t bn_start_server(char **tokens, Node *curr, CNode *child);
ssize_t bn_close_server(char **tokens, Node *curr, CNode *child);
ssize_t bn_send(char **tokens, Node *curr, CNode *child);
ssize_t bn_start_client(char **tokens, Node *curr, CNode *child);
/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd);
int ls_traversal(int, DIR *d, int, char * substring);
int contains_substring(DIR * d, char * substring, int depth);


/* BUILTINS and BUILTINS_FN are parallel arrays of length BUILTINS_COUNT
 */
static const char * const BUILTINS[] = {"echo", "=", "ls", "cd", "cat", "wc", "pwd", "kill", "ps", "start-server", "close-server", "send", "start-client"};
static const bn_ptr BUILTINS_FN[] = {bn_echo, bn_var, bn_ls, bn_cd, bn_cat, bn_wc, bn_pwd, bn_kill, bn_ps, bn_start_server, bn_close_server, bn_send, bn_start_client, NULL};    // Extra null element for 'non-builtin'
static const size_t BUILTINS_COUNT = sizeof(BUILTINS) / sizeof(char *);

void create_client(int client_socket);

struct listen_sock {
    struct sockaddr_in *addr;
    int sock_fd;
};

typedef struct clients {
    int client_fd;
    struct clients *next;
}client_node;
#endif
