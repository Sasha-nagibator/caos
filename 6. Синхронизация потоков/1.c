/*
Задача inf-III-06-1: posix/threads/condvar

Программа принимает три аргумента: два 64-битных числа A и B, и 32-битное число N.

Затем создается дополнительный поток, которые генерирует простые числа в диапазоне от A до B включительно, и сообщает об этом основному потоку, с которого началось выполнение функции main.

Главный поток выводит на стандартный поток вывода каждое полученное число и завершает свою работу после того, как получит N чисел.

Запрещено использовать глобальные переменные.
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <stdint.h>

struct task {
    uint64_t A;
    uint64_t B;
    pthread_mutex_t* mutex;
    pthread_cond_t* condvar_ready;
    pthread_cond_t* condvar_written;
    uint64_t p;
    uint64_t needed;
    int next_lock_taker;
};

void* generate(void* args) {
    struct task* arg = (struct task*)args;
    uint64_t i = 0, j = 0;
    int ready = 0;
    for (i = arg->A; i <= arg->B; ++i) {
        int flag = 1;

        for (j = 2; j * j <= i; ++j) {
            if (i % j == 0) {
                flag = 0;
                break;
            }
        }
        if (flag == 1) {

            pthread_mutex_lock(arg->mutex);
            arg->p = i;
            ready++;
            arg->next_lock_taker = 1;
            pthread_cond_signal(arg->condvar_ready);
            while (arg->next_lock_taker != 0) {
                pthread_cond_wait(arg->condvar_written, arg->mutex);
            }
            pthread_mutex_unlock(arg->mutex);
            if (ready == arg->needed) {
                pthread_cond_signal(arg->condvar_ready);
                return NULL;
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    uint64_t A = (uint64_t)strtol(argv[1], NULL, 10);
    uint64_t B = (uint64_t)strtol(argv[2], NULL, 10);
    uint32_t N = (uint32_t)strtol(argv[3], NULL, 10);
    pthread_t thread;

    pthread_mutex_t mutex;
    pthread_cond_t condvar_ready;
    pthread_cond_t condvar_written;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condvar_ready, NULL);
    pthread_cond_init(&condvar_written, NULL);
    struct task args;
    args.A = A;
    args.B = B;
    args.p = 0;
    args.mutex = &mutex;
    args.condvar_ready = &condvar_ready;
    args.condvar_written = &condvar_written;
    args.needed = N;
    args.next_lock_taker = 0;
    uint32_t i = 0;

    pthread_create(&thread, NULL, &generate, (void*)&args);

    for (i = 0; i < N; ++i) {

        pthread_mutex_lock(&mutex);
        while (args.next_lock_taker != 1) {
            pthread_cond_wait(&condvar_ready, &mutex);
        }

        pthread_mutex_unlock(&mutex);
        printf("%lu\n", args.p);
        args.next_lock_taker = 0;
        pthread_cond_signal(&condvar_written);
    }

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condvar_ready);
    pthread_cond_destroy(&condvar_written);
    return 0;
}

/*
Круто!
*/
