#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define READ_C 1
#define READ_A 0

void send_error_and_stop(char *massage, int code) {
    int i = strlen(massage);
    write(STDERR_FILENO, massage, sizeof(char) * i); 
    exit(code);
}

int main(int argc, const char *argv[]) {
    int length;
    int file_descriptors[2];
    file_descriptors[READ_A] = atoi(argv[1]);
    file_descriptors[READ_C] = atoi(argv[2]);

    while (1){   
        if (read(file_descriptors[READ_A], &length, sizeof(int)) == -1) {
            send_error_and_stop("Read from A error", 1);
        }

        if(length == 0) {
            break;
        }

        printf("from A = %d\n", length);

        if(read(file_descriptors[READ_C], &length, sizeof(int)) == -1) {
            send_error_and_stop("Read from C error", 2);
            return 1;
        }
        printf("from C = %d\n", length);
    }

    return 0;
}