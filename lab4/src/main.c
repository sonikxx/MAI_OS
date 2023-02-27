#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <wait.h>
#include <sys/stat.h>
#include <stdbool.h>

#define check_ok(VALUE, OK_VAL, MSG) if (VALUE != OK_VAL) { printf("%s", MSG); return 1; }
#define check_wrong(VALUE, WRONG_VAL, MSG) if (VALUE == WRONG_VAL) { printf("%s", MSG); return 1; }

const int FILENAME_LIMIT = 255;                                      //максимальная длина названия файла 255 на Ubuntu
const int BUFFER_SIZE = FILENAME_LIMIT;                              //длина под сообщение
const int SHARED_MEMORY_SIZE = BUFFER_SIZE + 1;                      //размер файла (можно сделать и больше)
const int STOP_FLAG = BUFFER_SIZE;

const char* SHARED_FILE_NAME = "meow";


bool check(const char* s, int len) {
    if (len < 2) return false;
    if (s[len - 2] != ';' && s[len - 2] != '.') return false;
    return true;                               
}

int last(const char* s) {                                          
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        if (s[i] == '\0') return i;
    }
    return BUFFER_SIZE;
}


int main(int argc, char** argv) {
    check_ok(argc, 2, "Specify the file name as the first argument\n")
    if (strlen(argv[1]) > FILENAME_LIMIT) {                             
        check_ok(1, 0, "Filename is too long\n")
    }
    int fd = shm_open(SHARED_FILE_NAME, O_RDWR | O_CREAT, S_IRWXU);   
    check_wrong(fd, -1, "Error creating shared file!\n")
    check_ok(ftruncate(fd, SHARED_MEMORY_SIZE), 0, "Error truncating shared file!\n")  

    char* map = (char*)mmap(NULL, SHARED_MEMORY_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);  
    check_wrong(map, NULL, "Cant map file\n")             

    
    const char* in_sem_name = "/input_semaphor";                                            //создание семафоров для синхронизации работы
    const char* out_sem_name = "/output_semaphor";
    
    sem_unlink(in_sem_name);                                                                //чтобы у нас был доступ, нужно его разлинковать, иначе ОС думает, что мы воруем чужой??   
    sem_unlink(out_sem_name);


    sem_t* in_sem = sem_open(in_sem_name, O_CREAT, S_IRWXU, 0);        
    check_wrong(in_sem, SEM_FAILED, "Cannot create 'in' semaphore\n")
    sem_t* out_sem = sem_open(out_sem_name, O_CREAT, S_IRWXU, 0);      
    check_wrong(out_sem, SEM_FAILED, "Cannot create 'out' semaphore\n")

    strcpy(map, argv[1]);                                                                  
    map[BUFFER_SIZE] = (char) strlen(argv[1]);                    

    int pid = fork();
    if (pid == -1) {
        check_wrong(pid, -1, "Fork failure\n")
    } else if (pid == 0) {
        //child
        int output_file = open(argv[1], O_RDWR | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE); 
        if (output_file == -1) {
            map[BUFFER_SIZE] = (char) STOP_FLAG;                                                 //чтобы сообщить родителю об ошибке 
            sem_post(out_sem);                                   
            check_ok(1, -1,"Cannot create output file\n")        
        }
        sem_post(out_sem);                                      
        while (true) {
            sem_wait(in_sem);                                     
            int l = (int) map[BUFFER_SIZE];                       
            if (check(map, l) == false ) {
                map[BUFFER_SIZE] = (char) STOP_FLAG;
                sem_post(out_sem);
                break;
            }
            check_wrong(write(output_file, map, map[BUFFER_SIZE]), -1, "Cannot write fo the file\n")
            sem_post(out_sem);
        }
        close(output_file);
    } else {
        //parent
        sem_wait(out_sem);                                                            //родительский процесс ждет, когда дочерний создаст файл
        if (map[BUFFER_SIZE] != (char) STOP_FLAG){    
            memset(map, 0, BUFFER_SIZE);    
            check_wrong(fgets(map, BUFFER_SIZE, stdin), NULL, "Unexpectedly EOF\n") 
            int read_count = last(map);       
            map[BUFFER_SIZE] = (char) read_count;
            sem_post(in_sem);
            while (true) {
                sem_wait(out_sem);
                if (map[BUFFER_SIZE] == (char) STOP_FLAG) {
                    break;
                }
                memset(map, 0, BUFFER_SIZE);
                check_wrong(fgets(map, BUFFER_SIZE, stdin), NULL, "Unexpectedly EOF\n")
                read_count = last(map);
                map[BUFFER_SIZE] = (char) read_count;
                sem_post(in_sem);
            }
            int stat_lock;                                                            //статус, с которым завершится дочерний процесс
            wait(&stat_lock);
            if (stat_lock != 0) {
                printf("Child failure\n");
            }
        } else {
            int stat_lock;
            wait(&stat_lock);
            if (stat_lock != 0) {
                printf("Child failure\n");
            }
        }

        sem_close(in_sem);
        sem_close(out_sem);
        check_wrong(munmap(map, SHARED_MEMORY_SIZE), -1, "Error unmapping\n")
        check_wrong(shm_unlink(SHARED_FILE_NAME), -1, "Error unlinking shared cond file!\n")
    }
}