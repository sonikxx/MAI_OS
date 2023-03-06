#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

void send_error_and_stop(char *massage, int code) {
    int i = strlen(massage);
    write(STDERR_FILENO, massage, sizeof(char) * i); 
    exit(code);
}

int main() {
    int file_descriptorsA_to_C[2];
    int file_descriptorsA_to_B[2];
    int file_descriptorsC_to_B[2];
    int file_descriptorsC_to_A[2]; 

    int pipeA_to_C = pipe(file_descriptorsA_to_C);
    int pipeA_to_B = pipe(file_descriptorsA_to_B);
    int pipeC_to_B = pipe(file_descriptorsC_to_B);
    int pipeC_to_A = pipe(file_descriptorsC_to_A);
    if (pipeA_to_C == -1 || pipeA_to_B == -1 || pipeC_to_B == -1 || pipeC_to_A == -1) {
        send_error_and_stop("Pipe error\n", 1);
    }

    switch (fork()) {
    case -1:
        send_error_and_stop("Fork error\n", 2);
        break;
    case 0:
        // B
        close(file_descriptorsA_to_C[WRITE]);
        close(file_descriptorsA_to_C[READ]);
        close(file_descriptorsC_to_B[WRITE]);
        close(file_descriptorsA_to_B[WRITE]);
        close(file_descriptorsC_to_A[WRITE]);
        close(file_descriptorsC_to_A[READ]);

        char file_descriptor_read_A[10];
        char file_descriptor_read_C[10];
        sprintf(file_descriptor_read_A, "%d", file_descriptorsA_to_B[READ]);
        sprintf(file_descriptor_read_C, "%d", file_descriptorsC_to_B[READ]);

        char *B_argv[] = {"b", file_descriptor_read_A, file_descriptor_read_C, NULL};

        if (execv("b", B_argv) == -1) {
            send_error_and_stop("Exec B error\n", 3);
        }
        break;
    default:
        switch (fork()) {
        case -1:
            send_error_and_stop("Fork error\n", 2);
            break;
        case 0:
            // C
            close(file_descriptorsA_to_C[WRITE]);
            close(file_descriptorsA_to_B[WRITE]);
            close(file_descriptorsA_to_B[READ]);
            close(file_descriptorsC_to_B[READ]);
            close(file_descriptorsC_to_A[READ]);

            char file_descriptor_read_A[10];
            char file_descriptor_write_A[10];
            char file_descriptor_write_B[10];
            sprintf(file_descriptor_read_A, "%d", file_descriptorsA_to_C[READ]);
            sprintf(file_descriptor_write_A, "%d", file_descriptorsC_to_A[WRITE]);
            sprintf(file_descriptor_write_B, "%d", file_descriptorsC_to_B[WRITE]);
            
            char *C_argv[] = {"c", file_descriptor_read_A, file_descriptor_write_A, file_descriptor_write_B, NULL};

            if (execv("c", C_argv) == -1) {
                send_error_and_stop("Exec C error\n", 4);
            }
            break;
        default:
            // A
            close(file_descriptorsA_to_C[READ]);
            close(file_descriptorsA_to_B[READ]);
            close(file_descriptorsC_to_B[WRITE]);
            close(file_descriptorsC_to_B[READ]);
            close(file_descriptorsC_to_A[WRITE]);
            
            char string[100];
            while (1) {
                if(gets(string) == NULL) {
                    send_error_and_stop("Gets error\n", 5);
                }
                int length = strlen(string);
                
                if(length == 0) {
                    if(write(file_descriptorsA_to_C[WRITE], &length, sizeof(length)) == -1) {
                        send_error_and_stop("Write in fdA_to_C error\n", 6);
                    }
                    if(write(file_descriptorsA_to_B[WRITE], &length, sizeof(length)) == -1) {
                        send_error_and_stop("Write in fdA_to_B error\n", 7);
                    }
                    break;
                }
                // write to C
                if(write(file_descriptorsA_to_C[WRITE], &length, sizeof(length)) == -1) {
                    send_error_and_stop("Write in fdA_to_C error\n", 6);
                }
                if(write(file_descriptorsA_to_C[WRITE], &string, sizeof(char) * length) == -1) {
                    send_error_and_stop("Write in fdA_to_C error\n", 6);
                }

                // write to B
                if(write(file_descriptorsA_to_B[WRITE], &length, sizeof(length)) == -1) {
                    send_error_and_stop("Write in fdA_to_B error\n", 7);
                }

                // read from C
                int check = 0;
                if(read(file_descriptorsC_to_A[READ], &check, sizeof(check)) == -1) {
                    send_error_and_stop("Read from fdA_to_C error\n", 8);
                    return 1;
                }
            }
            break;
        }
    }
    return 0;
}
