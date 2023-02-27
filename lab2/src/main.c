#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#define WRITE_END 1
#define READ_END 0
#define TRUE 1

void send_error_and_stop(char *massage, int code) {
    int i = 0;
    while (massage[i] != '\0') {
        i++;
    }
    write(STDERR_FILENO, massage, sizeof(char) * i); 
    exit(code);
}

int read_line(char **ptr) {                                                
    if (ptr == NULL) {                              
        return -1;
    }
    int capacity = 10;
    (*ptr) = malloc(sizeof (char) * capacity); 
    if ((*ptr) == NULL) {                                                      // cannot malloc memory
        return -1;
    }
    char *new_ptr;                                                             // tmp pointer to expand string in future
    char c;
    int i = 0;                                                                 // string size
    while (c != EOF && c !='\n') {
        if (read(STDIN_FILENO, &c, sizeof(char)) == -1) {
            send_error_and_stop("Cannot read symbol\n", 1);
            }
        if (c == '\n') break;
        (*ptr)[i++] = (char) c;                                               
        if (i >= capacity / 2) {                                               // expend string
            new_ptr = realloc((*ptr), sizeof (char) * capacity * 2);
            if (new_ptr == NULL) {                                             // cannot malloc new memory      
                return -1;
            }
            (*ptr) = new_ptr;                                              
            capacity *= 2;
        }
    }
    (*ptr)[i++] = '\0';
    return i; 
}

int main(int argc, const char *argv[]) {
    char *FileName;
    if (read_line(&FileName) == -1) {
        send_error_and_stop("Cannot read file name to open\n", 1);
    }

    int fd1[2];                                                                                                            
    int fd2[2];
    int p1 = pipe(fd1);      
    int p2 = pipe(fd2);
    if (p1 == -1 || p2 == -1) {
        send_error_and_stop("Pipe error\n", 1);
    }

    int fd;                                                                  
    if ((fd = open(FileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        send_error_and_stop("Cannot open file\n", 2);
    }
    
    free(FileName);
    FileName = NULL;
    
    int pid = fork();
    if (pid == -1) {
        send_error_and_stop("Fork error\n", 3);
    } else if (pid == 0) {
// =================  child  ================= //
        if (close(fd1[WRITE_END]) == -1 || close(fd2[READ_END]) == -1) {       // close fd that we won't use
            send_error_and_stop("Cannot close fd\n", 4);
        }

        char fd_file [10];                                                     // is strings to store (int) fd so 10 characters would be enough
        char fd_read [10];
        char fd_write [10];

        sprintf(fd_file, "%d", fd);
        sprintf(fd_read, "%d", fd1[READ_END]);
        sprintf(fd_write, "%d", fd2[WRITE_END]);

        char *Child_args[] = {"child", fd_file, fd_read, fd_write, NULL};
        if (execv("child", Child_args) == -1) {                                
            send_error_and_stop("Cannot call exec child\n", 5);
        }
    } else {
// =================  parent ================= //
        if (close(fd1[READ_END]) == -1 || close(fd2[WRITE_END]) == -1) {       // close fd that we won't use
            send_error_and_stop("Cannot close fd\n", 6);
        }

        int len;                                                               // length of input line
        int child_answer;   
        char *line;                                                            // user input
        while (TRUE) {
            len = read_line(&line);
            if (write(fd1[WRITE_END], &len, sizeof(int)) == -1) {
                send_error_and_stop("Cannot write to fd\n", 7);
            }
            if (len <3 - 1) {
                free(line);
                line = NULL;
                break;
            }
            if (write(fd1[WRITE_END], line, sizeof(char) * len) == -1) {
                send_error_and_stop("Cannot write to fd\n", 7);
            }
            if (read(fd2[READ_END], &child_answer, sizeof(int)) == -1) {
                send_error_and_stop("Cannot read from fd\n", 8);
            }
            if (child_answer == -1) {
                printf("%s has no \';\' or \'.\' in the end\n", line);
            }
            
            free(line);
            line = NULL;
        }

        if (close(fd1[WRITE_END]) == -1 || close(fd2[READ_END]) == -1 || close(fd) == -1) {                // close all fd because we end to  write/read at all
            send_error_and_stop("Cannot close fd\n", 9);
        }

        if (waitpid(pid, NULL, 0) == -1) {                                     // wait for child process end, pid = child id
            send_error_and_stop("Waiting error\n",10);
        }
        return 0;
        }
}
