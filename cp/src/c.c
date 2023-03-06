#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define WRITE_B 2
#define WRITE_A 1
#define READ_A 0

void send_error_and_stop(char *massage, int code) {
    int i = strlen(massage);
    write(STDERR_FILENO, massage, sizeof(char) * i); 
    exit(code);
}

int main(int argc, const char *argv[]) {
    int file_descriptors[3];
    file_descriptors[READ_A] = atoi(argv[1]);
    file_descriptors[WRITE_A] = atoi(argv[2]);
    file_descriptors[WRITE_B] = atoi(argv[3]);
    
    int length;
    char string[100];
    memset(string, 0, 100);
    
    while (1) {
        if(read(file_descriptors[READ_A], &length, sizeof(int)) == -1) {
            send_error_and_stop("Read from A error", 1);
        }

        if(length == 0) {
            break;
        }
        
        if(read(file_descriptors[READ_A], &string, sizeof(char) * length) == -1) {
            send_error_and_stop("Read from A error", 1);
        }

        printf("C: %s\n", string);

        if(write(file_descriptors[WRITE_A], &length, sizeof(int)) == -1) {
            send_error_and_stop("Write to A error", 2);
        }

        if(write(file_descriptors[WRITE_B], &length, sizeof(int)) == -1) {
            send_error_and_stop("Write to B error", 3);
        }
        memset(string, 0, 100);
        length = 0;
    }
    return 0;
}