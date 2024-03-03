/* pipe4.c
 *
 * CSC 360, Spring 2024
 *
 * Execute up to four instructions, piping the output of each into the
 * input of the next.
 *
 * Please change the following before submission:
 *
 * Author: Michael Schmidt
 * Login:  michaelschmidt@uvic.ca 
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_COMMANDS 4
#define BUFFER_SIZE 1024

int main() {

    char commands[MAX_COMMANDS][BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int num_commands = 0;
    int bytes_read;
    char* err_message;

    while(num_commands < MAX_COMMANDS){

        bytes_read = read(0, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 1){ // breaks if enter is pressed on an empty line
            break;
        }

        buffer[bytes_read - 1] = '\0'; // null terminate read string
        strncpy(commands[num_commands], buffer, BUFFER_SIZE);
        num_commands++;

    }

    if (num_commands < 1) return 0; // return if no commands entered

    
    // parse all command args
    char* args[num_commands][8];
    int pos;
    for (int i = 0; i < num_commands; i++){
        char* arg_tok = strtok(commands[i], " ");
        pos = 0;
        while(arg_tok != NULL){
            args[i][pos++] = arg_tok;
            arg_tok = strtok(NULL, " ");
        }
        args[i][pos] = NULL;
    }

    // make and fill pipe array
    int pipes[num_commands - 1][2];
    if (num_commands > 1){
        for (int i = 0; i < num_commands - 1; i++){
            if (pipe(pipes[i]) == -1){
                err_message = "ERROR: Failed to open pipe.\n";
                write(2, err_message, strlen(err_message));
                exit(0);
            }
        }
    }

    // execute all commands, piping the output of one into the input of the next one
    pid_t pids[num_commands];
    if (num_commands > 1){
        for (int i = 0; i < num_commands; i++){
            pids[i] = fork();
            
            if (pids[i] == -1){
                err_message = "ERROR: Failed to fork.\n";
                write(2, err_message, strlen(err_message));
                exit(0);
                
            }else if (pids[i] == 0){ // CHILD
                if (i > 0){ // if not the first command
                    dup2(pipes[i - 1][0], 0); // redirect stdin to the read end of last pipe
                }
    
                if (i < num_commands - 1){ // if not the last command
                    dup2(pipes[i][1], 1); // redirect stdout to the write end of the current pipe
                }

                // closing pipes in child
                for (int j = 0; j < num_commands - 1; j++) {
                    if (j != i - 1 || i == 0) close(pipes[j][0]); // close read ends of all pipes except the current read end needed; if it's for the first command, close ALL read ends.
                    if (j != i || i == num_commands - 1) close(pipes[j][1]); // close write ends of all pipes except the current write end needed; if it's for the last command, close ALL write ends. 
                }
                
                execvp(args[i][0], args[i]);
                err_message = "ERROR: Failed to execute command.\n";
                write(2, err_message, strlen(err_message));
                exit(0);
            }
        
        }

        int status;

        // close all pipes and wait for children
        for (int i = 0; i < num_commands; i++){
            if (i != num_commands - 1){
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            waitpid(pids[i], &status, 0);
        }

    }else{
        execvp(args[0][0], args[0]);
        err_message = "ERROR: Failed to execute command.\n";
        write(2, err_message, strlen(err_message));
        exit(0);
    }
    
}



