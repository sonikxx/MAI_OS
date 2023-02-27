#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;                                             

typedef struct ThreadToken {
    int** arr;                                                       
    int* res;                                                   
    int start;
    int steps;
    int K;                                                           //количество массивов
    int N;                                                           //длина массива
} ThreadToken;

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

void exit_with_msg(const char* message, int return_code) {
    printf("%s\n", message);
    exit(return_code);
}

void* vertical_sum_arrays(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    for (int i = token.start; i < token.start + token.steps; ++i) {
        int c = 0;
        for (int j = 0; j < token.K; ++j) {
            c += token.arr[j][i];
        }
        token.res[i] = c;
    }
    return arg;
}

void* horizontal_sum_arrays(void* arg) {
    ThreadToken token = *((ThreadToken*) arg);
    for (int i = 0; i < token.N; ++i) {
        int c = 0;
        for (int j = token.start; j < token.start + token.steps; ++j) {
            c += token.arr[j][i];
        }
        pthread_mutex_lock(&mutex);
        token.res[i] += c;
        pthread_mutex_unlock(&mutex);
    }
    return arg;
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        exit_with_msg("Missing arguments", 1);
    }
    int CountThreads = 0;
    for (int i = 0; argv[1][i] > 0; ++i) {
        if (argv[1][i] >= '0' && argv[1][i] <= '9') {
            CountThreads = CountThreads * 10 + argv[1][i] - '0';
        }
    }
    int N, K;
    printf("Length of arrays N: ");
    scanf("%d", &N);
    printf("Number of arrays K: ");
    scanf("%d", &K);
    int** all = malloc(sizeof(int*) * K);
    if (all == NULL) {
        exit_with_msg("Cannot allocate memory", 2);
    }
    for (int i = 0; i < K; ++i) {
        all[i] = malloc(sizeof(int) * N);
        if (all[i] == NULL) {                                        //если не получилось выделить память под очередной массив, то отчистим память
            for (int j = 0; j < i; ++j) {
                free(all[j]);
                all[j] = NULL;
            }
            free(all);
            all = NULL;
            exit_with_msg("Cannot allocate memory", 2);
        }
        for (int j = 0; j < N; ++j) {
            scanf("%d", &all[i][j]);
        }
    }

    void* function;
    int end;
    if (N > K * 2) {
        function = &vertical_sum_arrays;
        end = N;
        printf("vertical\n");
    } else {
        function = &horizontal_sum_arrays;
        end = K;
        printf("horizontal\n");
        if (pthread_mutex_init(&mutex, NULL) != 0) {                
            exit_with_msg("Cannot init mutex", 3);
        }
    }
    CountThreads = min(CountThreads, end);                           
    pthread_t* th = malloc(sizeof (pthread_t) * CountThreads);       //id потока
    ThreadToken* token = malloc(sizeof(ThreadToken) * CountThreads);
    int* result = malloc(sizeof(int) * N);
    if (th == NULL || token == NULL || result == NULL) {
        exit_with_msg("Cannot allocate memory", 2);
    }
    for (int i = 0; i < N; ++i) {
        result[i] = 0;
    }
    int start = 0;
    int steps = (end + CountThreads - 1) / CountThreads;             //округление вверх
    for (int i = 0; i < CountThreads; ++i) {
        token[i].arr = all;
        token[i].res = result;
        token[i].start = start;
        token[i].K = K;
        token[i].N = N;
        token[i].steps = min(end - start, steps);                    //выбираем минум из того, что осталось, и из того, что нужно
        start += steps;
    }

    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_create(&th[i], NULL, function, &token[i]) != 0) {          
            exit_with_msg("Cannot create thread", 4);
        }
    }
    for (int i = 0; i < CountThreads; ++i) {
        if (pthread_join(th[i], NULL) != 0) {
            exit_with_msg("Cannot join threads", 5);
        }
    }

    for (int i = 0; i < N; ++i) {                                 
        printf("%d ", result[i]);
    }
    printf("\n");
    for (int i = 0; i < K; ++i) {                                  
        free(all[i]);
        all[i] = NULL;
    }
    if (end == K) {
        pthread_mutex_destroy(&mutex);
    }
    free(all);
    free(token);
    free(th);
    free(result);
    return 0;
}