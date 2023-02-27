#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

int main(int argc, const char *argv[]) {
    if (argc < 4) {
        send_error_and_stop("Arguments missing\n", 1);
    }
    
    int fd;                                                                    // fd to file, read and write from argv[1], argv[2] and argv[3]
    int fd1[2];
    fd = atoi(argv[1]);
    fd1[READ_END] = atoi(argv[2]);
    fd1[WRITE_END] = atoi(argv[3]);

    if (dup2(fd, STDOUT_FILENO) == -1) {                                      //stdout <-> file                                   
        send_error_and_stop("Cannot do dup2\n", 2);
    }

    int  len;                                                                  // ans to parent process 
    char *str;
    int error_code = -1;
    while(TRUE) {

        if (read(fd1[READ_END], &len, sizeof (int)) == -1) {   
            send_error_and_stop("Cannot read from file\n", 3);
        }
        
        if (len < 2) break;                                                    // 0 or 1
        str = malloc(sizeof(char) * len);
        if (str == NULL) {
            send_error_and_stop("Cannot allocate memory\n", 4);
        }

        if (read(fd1[READ_END], str, sizeof (char) * len) == -1) {  
            send_error_and_stop("Cannot read from file\n", 5);
        }

        if (str[len - 2] == '.' || str[len - 2] == ';') {                      // string: .....[last][\0] index of last = len - 2
            if (printf("%s\n", str) == -1) {
                send_error_and_stop("Cannot write to file\n", 6);
            }

            if (write(fd1[WRITE_END], &len, sizeof(int)) == -1) {             
                send_error_and_stop("Cannot write to fd\n", 7);
            }

        } else {
            if (write(fd1[WRITE_END], &error_code, sizeof(int)) == -1) {      
                send_error_and_stop("Cannot write to fd\n", 8);
            }
        }

        free(str);
        str = NULL;
    }
    if (close(fd1[WRITE_END]) == -1 || close(fd1[READ_END]) == -1 || close(fd) == -1) {
        send_error_and_stop("Cannot close fd\n", 9);
    }
    return 0;
}