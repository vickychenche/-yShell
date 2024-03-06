#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#include "builtins.h"
#include "io_helpers.h"
#include "variables.h"
CNode *child_front = NULL;
int active_child = 0;
bool server_running = false;

void childHandler(int dummy){
  int cpid = waitpid(-1, NULL, WNOHANG);
  CNode *temp = child_front;
  while(temp != NULL){
    if(cpid == temp->pid){
      active_child  -= 1;
      temp->active = false;   
      char d[2] = {};
      display_message("[");
      sprintf(d,"%d", temp->child_number);
      display_message(d);
      display_message("]+  Done ");
      display_message(temp->command);
      display_message("\n");
      
    }
    temp = temp->next;
  }
}

void intHandler(int dummy) {
    display_message("\n");
    char * prompt = "mysh$ ";
    display_message(prompt);
}

int main(int argc, char* argv[]) {
    
    char *prompt = "mysh$ ";
    //char c[MAX_STR_LEN] = {};
    char input_buf[MAX_STR_LEN + 1];
    char input_buf_cpy[MAX_STR_LEN + 1];
    input_buf[MAX_STR_LEN] = '\0';
    char *token_arr[MAX_STR_LEN] = {NULL};
    char *prompt_arr[MAX_STR_LEN] = {"mysh$ "};
    char *background = "&";
    bool b = false;
    int num_of_child = 0;
    Node *front = NULL;
    //CNode *child_front = NULL; //child front
    signal(SIGINT, intHandler);
    signal(SIGCHLD, childHandler);
    child_front = create_child(getpid(), prompt_arr, child_front, 0);
    while (1) {
        // Prompt and input tokenization
        // Display the prompt via the display_message function.
        
        display_message(prompt);
        char *builtin_arr[MAX_STR_LEN][MAX_STR_LEN] = {};
        int ret = get_input(input_buf);
        if(strlen(input_buf) > 2){
          strncpy(input_buf_cpy, input_buf, strlen(input_buf)-2);
          input_buf[strlen(input_buf)] = '\0';
        }
        else{
          strncpy(input_buf_cpy, input_buf, MAX_STR_LEN);
          input_buf[strlen(input_buf)] = '\0';
        }
        size_t token_count = tokenize_input(input_buf, token_arr);
        
        // Clean exit
        //current problem control c also exits and exitt or exits all work
       
        if (ret != -1){
           
          if (token_count == 0){
            if ( !(ret >= 5) && ret != 1){
              if(server_running){
                server_running = false;
                bn_close_server(token_arr, NULL, NULL);
              }
              break;
            }

          }
          else if (strcmp("exit", token_arr[0]) == 0){
            
            if(server_running){
              server_running = false;
              bn_close_server(token_arr, NULL, NULL);
            }
            Node* temp;
            while (front != NULL){
              free(front->key);
              free(front->value);
              temp = front->next;
              free(front);
              front = temp;
            }
            CNode* ctemp;
            while(child_front != NULL){
              free(child_front->command);
              ctemp = child_front->next;
              free(child_front);
              child_front = ctemp;
            }
            
            break;
          }
        }
        int r = 0;
        if (token_count != 0){
        int length = strlen(token_arr[token_count-1]);
        
          if(strncmp(token_arr[token_count-1], background, MAX_STR_LEN) == 0){
            
            num_of_child ++;
            token_arr[token_count-1] = '\0';
            token_count -= 1;
            r = fork();
            if (r == 0){
              b = true;
            }
          }
          else if(strncmp(token_arr[token_count-1]+length-1, background, MAX_STR_LEN) == 0){ //matches completely 
            num_of_child ++;
            char * temp = strtok(token_arr[token_count-1] ,background);
            token_arr[token_count-1] = temp;
            token_count -= 1;
            r = fork();
            if(r == 0){
              b = true;
            }
          }
          /*
          else if(strncmp(token_arr[0], "start-server", MAX_STR_LEN) == 0){
            server_running = true;
            r = fork();
            
            if (r>0){
              //save the value so when i close server i can kill this one
            }
          }
          */
          
        }
        
        if (r < 0){
          perror("fork");
          return -1;
        }else if (r == 0){
          sleep(0.005);

        
        if (token_count >= 1){
          // check if contains pipe here
          int num_pipe = 0;
          char * p = "|";
          int pos = -1;
          int pos2 = 0;
          bool added = false;
          for (int i = 0; i < token_count; i ++){
            if(!added){ //false
              //pipe_pos[pos] = i;
              pos ++;
              added = true;
            }
            if (strncmp(token_arr[i], p, MAX_STR_LEN) == 0){ //if its the same |
              num_pipe ++;
              added = false;
              pos2 = 0;
            }
            else{
              builtin_arr[pos][pos2] = token_arr[i];
              pos2 ++;
              //display_message(builtin_arr[pos][pos2]);
            }
          }
          
          bn_ptr builtin_fn; // = check_builtin(token_arr[0]);
          ssize_t err; // = 0;
          //if (builtin_fn != NULL){
            
            if(num_pipe != 0){ // if contains pipe
              
              int pipe_fd[num_pipe][2];
              
              
              pos = 0;
              int r;
              for(int _ = 0; _ <= num_pipe; _ ++){ //1 pipe 2 child etc
                if(_ != num_pipe){
                  if(pipe(pipe_fd[_]) == -1){ //have to create pipe before forking
                    perror("pipe");
                    return -1;
                  }
                }
                
                r = fork();
                if(r < 0){
                  perror("fork");
                  return -1;
                }else if(r == 0){ //child process
                  
                  if(_ != num_pipe){ //if its not the last child

                    if(dup2(pipe_fd[_][1], STDOUT_FILENO) == -1){ //change the stdout to writing of curr pipe
                      perror("dup2 1 error");
                      return -1;
                    }
                    
                  }
                  
                  if(_ != 0){ //if its not the first child
                    
                    if(dup2(pipe_fd[_-1][0], STDIN_FILENO) == -1){ //change the stdin to the readin of last pipe
                      perror("dup2 error");
                      return -1;
                    }
                    
                    if(close(pipe_fd[_-1][0]) == -1){
                      perror("child close reading end");
                      return -1;
                    }
                    if(close(pipe_fd[_-1][1]) == -1){
                      perror("child close writing end");
                      return -1;
                    }
                    
                  }
                  if(_ != num_pipe){
                    if(close(pipe_fd[_][1]) == -1){
                      perror("child close writing end");
                      return -1;
                    }
                    if(close(pipe_fd[_][0]) == -1){
                        perror("child close reading end");
                        return -1;
                    }
                  }
                  builtin_fn = check_builtin(builtin_arr[_][0]);
                  if(builtin_fn != NULL){
                    if (builtin_fn == bn_var){
                      if ((builtin_arr[_][1] != NULL)){
                        display_error("ERROR: Unrecognized command: ", builtin_arr[_][0]);
                      }
                      else{
                        front = new_node(builtin_arr[_], front);
                      }
                    }
                    else{
                      err = builtin_fn(builtin_arr[_], front, child_front);
                      if (err == - 1) {
                        display_error("ERROR: Builtin failed: ", builtin_arr[_][0]);
                      }
                    }
                    
                  }else{
                    if(execvp(builtin_arr[_][0], builtin_arr[_]) == -1){
                      display_error("ERROR: Unrecognized command: ", builtin_arr[_][0]);
                    }
                  }
                  
                  //bug wrong command first | good command
                  //prints out nothing
                  /*
                  if(server_running){
                    //display_message("here");
                    bn_close_server(token_arr, NULL, NULL);
                  }
                  */
                  //free_all(front, child_front);
                  exit(0);
                }
                else if(r > 0){ //parent process
                  
                  //dup2(pipe_fd[_][0], STDIN_FILENO); //rewrite read to stdin
                  //if ( _ != 0){
                    if(_ != 0){
                      if(close(pipe_fd[_-1][0]) == -1){
                        perror("parent close reading end");
                        return -1;
                      }
                      if(close(pipe_fd[_-1][1]) == -1){
                        perror("parent close writing end");
                        return -1;
                      }
                    }
                    
                  //}
                  
                }
              }
              

              for (int i = 0; i <= num_pipe; i ++){
                    if(wait(NULL) < 0){
                      perror("wait");
                      exit(1);
                    }
              }
            }
            else{ // Command execution
            
              builtin_fn = check_builtin(token_arr[0]); 
              if(builtin_fn != NULL){
                if (builtin_fn == bn_var){
                  if (token_count != 1){
                    display_error("ERROR: Unrecognized command: ", token_arr[0]);
                  }
                  else{
                    front = new_node(token_arr, front);
                  }
                }else if(builtin_fn == bn_close_server){
                  server_running = false;
                  err = builtin_fn(token_arr, front, child_front);
                }
                else{
                  
                  err = builtin_fn(token_arr, front, child_front);
                }

                if (err == - 1) {
                  display_error("ERROR: Builtin failed: ", token_arr[0]);
                }
              }
              else{
                int r = fork();
                if(r == 0){
                  if(execvp(token_arr[0], token_arr) == -1){
                    display_error("ERROR: Unrecognized command: ", token_arr[0]);
                  }
                  exit(0);
                }
                if (r > 0 ){
                  wait(NULL);
                }
              }
              
            }
            
        }
        if(b){
          b = false;
          num_of_child -= 1;
          exit(0);
        }
        
      }
      else if (r > 0){
          CNode *temp = child_front;
          int c = 0;
          if(strncmp(token_arr[0], "start-server", MAX_STR_LEN) != 0){
            while (temp != NULL){
              if(temp->active){
                c += 1;
              }
              temp = temp->next;
            }
            active_child = c;
            child_front = create_child(r, token_arr, child_front, active_child); //need to add a loop to check

            display_message("[");
            char b[MAX_STR_LEN] = {};
            sprintf(b, "%d", active_child);
            display_message(b);
            display_message("] ");
            char d[MAX_STR_LEN] = {};
            sprintf(d, "%d", r);
            display_message(d);
            display_message("\n");
          }
          
          
      }
    }
    //need to free CNode as well
    Node* temp;
  while (front != NULL){
    free(front->key);
    free(front->value);
    temp = front->next;
    free(front);
    front = temp;
  }
  CNode* ctemp;
  while(child_front != NULL){
    free(child_front->command);
    ctemp = child_front->next;
    free(child_front);
    child_front = ctemp;
  }
    return 0;
}
