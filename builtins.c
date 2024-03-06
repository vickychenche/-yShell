#include <string.h>
#include <dirent.h>
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <netinet/in.h>    /* Internet domain header */
#include <netdb.h>
#include <arpa/inet.h> 
#include <unistd.h>

#include "io_helpers.h"
#include "variables.h"
#include "commands.h"

#include <arpa/inet.h>
#include <sys/types.h>

int server_pid = 0;
struct listen_sock s;
//struct sockaddr_in server;
client_node *clients = NULL;
//LNode *client_nodes = NULL;
int listen_soc;
//should create a link list for client
int sigint_received = 0;
void sigint_handler(int code) {
    sigint_received = 1;
}
char *sign = "$";
// ====== Command execution =====

/* Return: index of builtin or -1 if cmd doesn't match a builtin
 */
bn_ptr check_builtin(const char *cmd) {
    ssize_t cmd_num = 0;
    while (cmd_num < BUILTINS_COUNT &&
           strncmp(BUILTINS[cmd_num], cmd, MAX_STR_LEN) != 0 ) {
        if (cmd_num == 1 && strchr(cmd, *BUILTINS[cmd_num]) != NULL){
          return BUILTINS_FN[cmd_num];
        }
        cmd_num += 1;
    }


    return BUILTINS_FN[cmd_num];
}

// ===== Builtins =====

/* Prereq: tokens is a NULL terminated sequence of strings.
 * Return 0 on success and -1 on error ... but there are no errors on echo.
 */

ssize_t bn_echo(char **tokens, Node *curr, CNode *child_front) {
    ssize_t index = 1;
    char *sign = "$";
    if (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        if (strchr(tokens[index], *sign)){
          char **temp = &tokens[index];

          if (bn_var(temp, curr, child_front) == NOT_PRINTED){
            display_message(tokens[index]);
          }
        }
        else{
          display_message(tokens[index]);
        }
        index += 1;
    }
    while (tokens[index] != NULL) {
        // TODO:
        // Implement the echo command
        display_message(" ");
        if (strchr(tokens[index], *sign)){

          char **temp = &tokens[index];
          if (bn_var(temp, curr, child_front) == NOT_PRINTED){
            display_message(tokens[index]);
          }
        }
        else{

          display_message(tokens[index]);
        }
        index += 1;
    }
    display_message("\n");

    return 0;
}

ssize_t bn_var(char **tokens, Node *curr, CNode *child_front){
  Node *temp = curr;
  ssize_t index = 0;
  bool Not_printed = true;
  char *toke = tokens[index];
  if (strlen(toke) == 1){
    return NOT_PRINTED;
  }
  toke = tokens[index] + sizeof(char);
  char place[strlen(toke)+1];
  strncpy(place, toke, strlen(toke) + sizeof(char));
  while(temp != NULL){
    if (strncmp(temp->key, place, strlen(place)) == 0){
      display_message(temp->value);
      Not_printed = false;
    }
    temp = temp->next;
  }

  if (Not_printed){
    return NOT_PRINTED;
  }
  return 0;
}

ssize_t bn_ls(char **tokens, Node *curr, CNode *child_front){
  DIR *d;
  //struct dirent *dir;
  char * f = "--f";
  char * rec = "--rec";
  char * depth = "--d";
  char substring[MAX_STR_LEN];
  char path[MAX_STR_LEN];
  long depth_value = 0;
  bool has_rec = false;
  bool has_depth = false;
  bool has_f = false;
  char *sign = "$";
  strncpy(path, ".", sizeof(char)*2);
  strncpy(substring, "", sizeof(char)*2);
  if (tokens[1] == NULL){ //if no input in provided
    d = opendir(path);
    if (d) {
      read_dir(d, 0);
      closedir(d);
    }
    else{
        display_error("ERROR: ", "Invalid path");
        closedir(d);
        return -1;
    }
  }
  else{
    for (size_t i = 1; tokens[i] != NULL; i++) {
      if (strncmp(tokens[i], f, strlen(f)) == 0 ){
        has_f = true;
        if (tokens[i+1] == NULL || strncmp(tokens[i+1], rec, strlen(rec)) == 0 || strncmp(tokens[i+1], depth, strlen(depth)) == 0){
          display_error("ERROR: ", "No substring provided");
          return -1;
        }
        else{
          strncpy(substring, tokens[i + 1], strlen(tokens[i+1]) + sizeof(char));
          i ++;
        }
        
      }
      else if(strncmp(tokens[i], rec, strlen(rec)) == 0){
        has_rec = true;
        if (tokens[i+1] == NULL || strncmp(tokens[i+1], f, strlen(f)) == 0 || strncmp(tokens[i+1], depth, strlen(depth)) == 0){
          strncpy(path, ".", sizeof(char)*2);
        }
        else{
          strncpy(path, tokens[i+1], sizeof(char)*(strlen(tokens[i+1])+1));
          i ++;
        }
        
      }
      else if (strncmp(tokens[i], depth, strlen(depth)) == 0){
        has_depth = true;
        if (tokens[i+1] == NULL || strncmp(tokens[i+1], f, strlen(f)) == 0 || strncmp(tokens[i+1], rec, strlen(rec)) == 0){
          display_error("ERROR: ", "No depth provided");
          return -1;
        }
        else{//problem
          char *temp = tokens[i+1];
          if(isdigit(*temp)){
            depth_value = strtol(temp, NULL, 10);
          }
          else{
            display_error("ERROR: ", "Invalid Depth Provided");
            return -1;
          }
          //what if its not a number
        }
        i ++;
      }
      else{
        strncpy(path, tokens[i], sizeof(char)*(strlen(tokens[i])+1));
      }
    }
    //d = opendir(tokens[1]);]
    if(has_rec && has_depth){ //has only 2
      if(strchr(path, *sign)){ //check if this path is a variable
        char *t = get_var(curr, path); //return if it is

        d = opendir(t);
        if (d){
          chdir(t);
        }
      }
      else{ //open if variable not exist
        d = opendir(path);
        if (d){
          chdir(path);
        }
      }
      if (d){ //should change the directory instead of opening
        char *g = getcwd(NULL, 64);
        ls_traversal(depth_value, d, 0, substring); //should take a substring and then print it out by a helper
        chdir(g);
        free(g);
        closedir(d);
      }
      else{
        closedir(d);
        display_error("ERROR: ", "Invalid path");
        return -1;
      }
      
    }//need one for none of everythign no rec , f , depth_value
    else if(has_f && !has_rec && !has_depth){
      if(strchr(path, *sign)){ //check if this path is a variable
        char *t = get_var(curr, path);

        d = opendir(t);
      }
      else{
        d = opendir(path);
      }
      if (d){
        read_dir_substring(d, substring);
      }
      else{
        closedir(d);
        display_error("ERROR: ", "Invalid path");
        return -1;
      }
      closedir(d);
      //one for ls --f substring
      //one for ls path --f substring
      //one for ls var --f substring
    }
    else if(!has_f && !has_rec && !has_depth){
      if(tokens[2] != NULL){
        display_error("ERROR: ", "Invalid path");
        return -1;
      }
      else{
        d = opendir(tokens[1]);
        if (d) {
          read_dir(d, 0);
          closedir(d);
        }
        else{
          strncpy(path, tokens[1], strlen(tokens[1]) + sizeof(char));
          if(strchr(path, *sign)){
            char * t = get_var(curr, path);
            d = opendir(t);
            if (d) {
              read_dir(d, 0);
              closedir(d);
            }
            else{
              closedir(d);
              display_error("ERROR: ", "Invalid path");
              return -1;
            }
          }
          else{
            display_error("ERROR: ", "Invalid path");
            return -1;
          }

        }
      }
    }
    //another else statment if has_f then call helper function and you know this
    //shoud be infront of the else statement above
  }

  return 0;
}



ssize_t bn_pwd(char **tokens, Node*curr, CNode *child_front){
  char *g = getcwd(NULL, 64);
  display_message(g);
  display_message("\n");
  free(g);
  return 0;
}

int ls_traversal(int depth_value, DIR * d, int times, char * substring){ 
//base case if depth value is 0
  struct dirent *dir;
  DIR *x;
  if(depth_value == 0){
    read_dir_substring(d, substring);
    return 0;
    
  }
  else{
    while ((dir = readdir(d)) != NULL) {
      if (strncmp(dir->d_name, ".", 1) != 0){
        
        x = opendir(dir->d_name);
        if ((strstr(dir->d_name, substring))){
          display_message(dir->d_name);
          display_message("\n");
          
        } //check if it is open able first
        if (x!= NULL){
            chdir(dir->d_name);
            ls_traversal(depth_value-1, x, times + 1, substring);
            chdir("..");
        }
          
          
        
        closedir(x);
        
      }
    else{
      if ((strstr(dir->d_name, substring))){
      display_message(dir->d_name);
      display_message("\n");
      }
    }
    }

  }
  return 0;
}
ssize_t bn_cd(char **tokens, Node *curr, CNode *child_front){
  int x = -1;
  char * l = "/";
  char * dot = ".";
  //need to make ..... work somehow
  //cd /
  //cd ~ and cd
  if (tokens[1] == NULL){
    chdir(l);
    return 0;
  }
  if (strchr(tokens[1], *l) == NULL && strchr(tokens[1], *dot) != NULL){
    int len = strlen(tokens[1]);
    if (len <= 2){
      x = chdir(tokens[1]);
    }
    else{
      bool contains = false;
      char *token = tokens[1];
      for (size_t i = 0; i < len; i++){
        if (strchr(&token[i], *dot) == NULL){
          contains = true;
        }
      }
      char path[MAX_STR_LEN] = "..";
      //char *path = malloc(sizeof(char)*3*(len-1)+1);
      //path = "..";
      char add_on[4] = "/..";
      if (!contains){
        for (size_t i = 2; i < len; i++) {
          strncat(path, add_on, 4);
        }
        x = chdir(path);
      }
      else{
        x = chdir(tokens[1]);
      }

      //free(path);
    }
  }
  else{
    x = chdir(tokens[1]);
  }
  if (x == -1){
    //error
    display_error("ERROR: Invalid path ", tokens[1]);
    return -1;
  }
  //else{
  //  char *g = getcwd(NULL, 64);
  //  display_message(g);
  //  display_message("\n");
  //}
  //chdir man page 2 man 2 chdir
  //
  return 0;
}

ssize_t bn_cat(char **tokens, Node *curr, CNode *child_front){
  //int error;
  FILE *file;
  int s;
  char c[2];
  //char *buffer = malloc(sizeof(int));
  if (tokens[1] != NULL){
    char *filename = tokens[1];
    file = fopen(filename, "r");
    if (file == NULL){
      
      display_error("ERROR: Cannot open file ", tokens[1]);
      //fclose(file);
      return -1;
    }
    else{
      do {
        s = fgetc(file);
        if (feof(file)){
          break;
        }
        //printf("%c",s);
        //fflush(stdout);
        //snprintf(buffer, sizeof(int), "%d", s);
        sprintf(c,"%c", s);
        display_message(c);
      }while(1);

    }
    fclose(file);
    return 0;
  }
  else{
    FILE *fptr = NULL;
    struct pollfd fds;
    fds.fd = 0;
    fds.events = POLLIN;
    int ret = poll(&fds, 1, 10);
    if(ret == 0){ //nothing to read from stdin
      
      display_error("ERROR: ", "No input source provided");
      //free(c);
      //free(buffer);
      return -1;
    }
    else{
      fptr = stdin;
      s = fgetc(fptr);
      while (s != EOF){
        sprintf(c,"%c", s);
        display_message(c);
        s = fgetc(fptr);
      }
      return 0;
        //printf("%c",s);
        //fflush(stdout);
        //snprintf(buffer, sizeof(int), "%d", s);
        
    }
  }



}

ssize_t bn_wc(char **tokens, Node *curr, CNode *child_front){
  int word_count = 0;
  int line_count = 0;
  int character_count = 0;
  bool on = false;
  FILE *file;
  int s;
  char *c = malloc(sizeof(char));
  char *buffer = malloc(sizeof(int));
  if (tokens[1] != NULL){
    char *filename = tokens[1];
    file = fopen(filename, "r");

    if (file == NULL){
      display_error("ERROR: Cannot open file ", tokens[1]);
      //fclose(file);
      free(c);
      free(buffer);
      return -1;
    }
    else{
      do {
        s = fgetc(file);
        if (feof(file)){
          break;
        }
        if (s == 10){
          //display_message("new line");
          line_count += 1;
          character_count += 1;
          if (on){
            on = false;
            word_count += 1;
          }

        }
        else if(s == 32 || s == 9){
          //display_message("white space");
          character_count += 1;
          if (on){
            word_count += 1;
            on = false;
          }
        }
        else{
          character_count += 1;
          on = true;
        }
      }while(1);
      *c = word_count;
      display_message("word count ");
      snprintf(buffer, sizeof(int), "%d", word_count);
      display_message(buffer);
      display_message("\n");
      display_message("character count ");
      snprintf(buffer, sizeof(int), "%d", character_count);
      display_message(buffer);
      display_message("\n");
      snprintf(buffer, sizeof(int), "%d", line_count);
      display_message("newline count ");
      display_message(buffer);
      display_message("\n");

    }
    fclose(file);
    free(c);
    free(buffer);
    return 0;
    //check if closing caused an error
  }
  else{
    //check if there is from read
    FILE *fptr = NULL;
    struct pollfd fds;
    fds.fd = 0;
    fds.events = POLLIN;
    int ret = poll(&fds, 1, 10);
    if(ret == 0){ //nothing to read from stdin
      
      display_error("ERROR: ", "No input source provided");
      free(c);
      free(buffer);
      return -1;
    }else{
      fptr = stdin;
      s = fgetc(fptr);
      while (s != EOF){
         if (s == 10){
          //display_message("new line");
          line_count += 1;
          character_count += 1;
          if (on){
            on = false;
            word_count += 1;
          }

        }
        else if(s == 32 || s == 9){
          //display_message("white space");
          character_count += 1;
          if (on){
            word_count += 1;
            on = false;
          }
        }
        else{
          character_count += 1;
          on = true;
        }
        s = fgetc(fptr);
      }
      *c = word_count;
      display_message("word count ");
      snprintf(buffer, sizeof(int), "%d", word_count);
      display_message(buffer);
      display_message("\n");
      display_message("character count ");
      snprintf(buffer, sizeof(int), "%d", character_count);
      display_message(buffer);
      display_message("\n");
      snprintf(buffer, sizeof(int), "%d", line_count);
      display_message("newline count ");
      display_message(buffer);
      display_message("\n");
      free(c);
      free(buffer);
    }
    return 0;
    
  }
}

ssize_t bn_kill(char **tokens, Node *curr, CNode *child_front){
  int ret = 0;
  int pid = 0;
  int sig = 0;
  if(tokens[1] != NULL){
    pid = strtol(tokens[1], NULL, 10);
    if(tokens[2] != NULL){
      if(strchr(tokens[2], *sign)){ //check if this path is a variable
        char *t = get_var(curr, tokens[2]);
        if(t == NULL){
          return -1;
        }
        else{
          sig = strtol(t, NULL, 10);
          ret =  kill(pid, sig);
        }
        
      }
      else{
        sig = strtol(tokens[2], NULL, 10);
        ret =  kill(pid, sig);
      }
      
    }
    else{
      ret = kill(pid, 1);
    }

    if(ret == -1){
      if(errno == ESRCH){
        display_error("ERROR: ", "The process does not exist");
        return -1;
      }
      else if(errno == EINVAL){
        display_error("ERROR: ", "Invalid signal specified");
        return -1;
      }
    }
    
    
  }
  else{
    return -1;
  }
  
  return 0;
}
ssize_t bn_ps(char **tokens, Node *curr, CNode *child_front){
  CNode *temp = child_front;
  char prompt[MAX_STR_LEN] = "./mysh";
  while (temp != NULL){
    if (strncmp(temp->command, prompt, MAX_STR_LEN) != 0){
      display_message(temp->command);
      display_message(" ");
      char d[MAX_STR_LEN] = {};
      sprintf(d,"%d", temp->pid);
      display_message(d);
      display_message("\n");
    }
    
    temp = temp->next;
  }
  return 0;
}

ssize_t bn_start_server(char **tokens, Node *curr, CNode *child){
  int r = fork();
  if (r == 0){
      int port = 0 ;
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
          perror("signal");
          exit(1);
      }
    
    if (tokens[1] != NULL){
      if(strchr(tokens[1], *sign)){
        char *t = get_var(curr, tokens[1]);
        if(t == NULL){
          port = strtol(tokens[1], NULL, 10);
        }
        else{
          port = strtol(t, NULL, 10);
        }
            
      }
      else{
        port = strtol(tokens[1], NULL, 10);
      }
      (&s)->addr = malloc(sizeof(struct sockaddr_in));
      (&s)->addr->sin_family = AF_INET;
      (&s)->addr->sin_port = htons(port);
      memset(&(&s)->addr->sin_zero, 0, 8);
      (&s)->addr->sin_addr.s_addr = INADDR_ANY;

      (&s)->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
      if ((&s)->sock_fd == -1) {
          display_error("ERROR: ", "server socket");
          exit(1);
      }


      int on = 1;
      int status = setsockopt((&s)->sock_fd, SOL_SOCKET, SO_REUSEADDR,(const char *) &on, sizeof(on));
      if (status == -1) {
          display_error("ERROR: ", "setsockopt -- REUSEADDR");
          exit(1);
      }

      // bind the selected port to socket
      if (bind((&s)->sock_fd, (struct sockaddr *) (&s)->addr, sizeof(*((&s)->addr))) == -1) {
        display_error("ERROR: ", "server: bind"); //this is the port we pick is not avaliable need to know like when obind doesnt work
        close((&s)->sock_fd);
        return -1;
      }

      if (listen((&s)->sock_fd, 1) < 0) {
          display_error("ERROR: ", "server: listen");
          close((&s)->sock_fd);
          return -1;
      }


      struct sigaction sa_sigint;
      memset (&sa_sigint, 0, sizeof (sa_sigint));
      sa_sigint.sa_handler = sigint_handler;
      sa_sigint.sa_flags = 0;
      sigemptyset(&sa_sigint.sa_mask);
      sigaction(SIGINT, &sa_sigint, NULL);

      
      struct sockaddr_in client_addr;
      unsigned int client_len = sizeof(struct sockaddr_in);
      client_addr.sin_family = AF_INET;
      fd_set all_fds, listen_fds;
      FD_ZERO(&all_fds);
      FD_SET(s.sock_fd, &all_fds);
      while(1){
      
        listen_fds = all_fds;
        int nready = select(1000 /*listen_soc+1*/, &listen_fds, NULL, NULL, NULL);
        if (sigint_received) break;
        if(nready < 0) {
            display_error("ERROR: ", "server: select");
            return -1;
        }
        
        if(FD_ISSET(s.sock_fd, &listen_fds)){
          int client_socket = accept(s.sock_fd, (struct sockaddr *)&client_addr, &client_len);
          if (client_socket == -1) {
              display_error("ERROR: ", "server: accpet");
              return -1;
          }
          FD_SET(client_socket, &all_fds);
          create_client(client_socket);
          //display_message("outputputputp");
          
        }
        
        if (sigint_received) break;
        
        client_node *temp = clients;
        char line[MAX_STR_LEN];
        while(temp != NULL){
          
          if (!FD_ISSET(temp->client_fd, &listen_fds)) {
              temp = temp->next;
              continue;
              
                  
          }
          if(read(temp->client_fd, line, MAX_STR_LEN) != 0){
            display_message("\n");
            display_message(line);
            display_message("\n");
            display_message("mysh$ ");
            }
          
          
          temp = temp->next;
        }
      }
    }
    else{
      return -1;
    }
  }
  else if(r>0){
    if (server_pid != 0){
      server_pid = r;
    }
  }
  
  //bn_close_server(tokens, curr, child);

  return 0;
  }

void create_client(int client_socket){
  
  client_node *new_node = malloc(sizeof(client_node));
  new_node->client_fd = client_socket;
  
  new_node->next = clients;
  clients = new_node;
  
}
ssize_t bn_close_server(char **tokens, Node *curr, CNode *child){
  client_node *temp = clients;
  while(temp != NULL){
    close(temp->client_fd);
    temp = temp->next;
  }

  //close(s.sock_fd);
  kill(server_pid, 0);
  return 0;
}
ssize_t bn_send(char **tokens, Node *curr, CNode *child){
  int i = 3;
  int port = 0;
  char input_buf[MAX_STR_LEN+1];
  char space[MAX_STR_LEN] = " ";
  while(tokens[i] != NULL){
    if(strchr(tokens[i], *sign)){
      char *t = get_var(curr, tokens[i]);
      if(t != NULL){
        strncat(input_buf, t, strlen(tokens[i]));
        strncat(input_buf, space, strlen(space));
      }
      else{
        strncat(input_buf, tokens[i], strlen(tokens[i]));
        strncat(input_buf, space, strlen(space));
      }
    }
    else{
      strncat(input_buf, tokens[i], strlen(tokens[i]));
      strncat(input_buf, space, strlen(space));
    }
    
    i ++;
  }
  if (tokens[1] == NULL){
    display_error("ERROR: ", "No port provided");
    return -1;
  }
  else if(tokens[2] == NULL){
    display_error("ERROR: ", "No hostname provided");
    return -1;
  }
  if(strchr(tokens[1], *sign)){
        char *t = get_var(curr, tokens[1]);
        if(t == NULL){
          port = strtol(tokens[1], NULL, 10);
        }
        else{
          port = strtol(t, NULL, 10);
        }
        
  }
  else{
    port = strtol(tokens[1], NULL, 10);
  }
  int soc = socket(AF_INET, SOCK_STREAM, 0);
  if (soc == -1) {
      display_error("ERROR ", "client: socket");
      return -1;
  }
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  memset(&server.sin_zero, 0, 8);

  struct addrinfo *result;
  if(strchr(tokens[2], *sign)){ //check if this path is a variable
        char *t = get_var(curr, tokens[1]);
        if(t == NULL){
          getaddrinfo(t, NULL, NULL, &result);
        }
        else{
          getaddrinfo(t, NULL, NULL, &result);
        }
        
      }
  else{
    getaddrinfo(tokens[2], NULL, NULL, &result);
  }
  

  server.sin_addr = ((struct sockaddr_in *) result->ai_addr)->sin_addr;

  freeaddrinfo(result);
  int ret = connect(soc, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  if (ret == -1){
    display_error("ERROR: ", "no server running");
    return -1;
  }
  write(soc, input_buf, MAX_STR_LEN);
  close(soc);
  /*
  
  write((&s)->sock_fd, input_buf, strlen(input_buf));
  */
  return 0;
}
ssize_t bn_start_client(char **tokens, Node *curr, CNode *child){
  int port = 0;
  char input_buf[MAX_STR_LEN + 1];
  if (tokens[1] == NULL){
    display_error("ERROR: ", "No port provided");
    return -1;
  }
  else if(tokens[2] == NULL){
    display_error("ERROR: ", "No hostname provided");
    return -1;
  }
  
  //int soc = socket(AF_INET, SOCK_STREAM, 0);
  port = strtol(tokens[1], NULL, 10);
  int soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1) {
        display_error("ERROR ", "client: socket");
        return -1;
    }
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  memset(&server.sin_zero, 0, 8);

  struct addrinfo *result;
  getaddrinfo(tokens[2], NULL, NULL, &result);

  server.sin_addr = ((struct sockaddr_in *) result->ai_addr)->sin_addr;

  freeaddrinfo(result);
  int ret = connect(soc, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  if (ret == -1){
    display_error("ERROR: ", "no server running");
    return -1;
  }
  while(1){
    get_input(input_buf);
    write(soc, input_buf, MAX_STR_LEN);
  }
  //server.sin_addr = ((struct sockaddr_in *) result->ai_addr)->sin_addr;
  
  //create_client(client_socket, client_addr);

  return 0;
}