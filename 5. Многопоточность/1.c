/*
Задача inf-III-05-1: posix/threads/parallel-sum

Единственным аргументом программы является целое число N>1 - число потоков, которые нужно создать.

На стандартном потоке ввода задается последовательность целых чисел.

Реализуйте программу, которая запускает N потоков, каждый из которых читает числа со стандартного потока ввода, и вычисляет частичные суммы.

На стандартный поток вывода необходимо вывести итоговую сумму всех чисел.

Минимизируйте объем используемой памяти настолько, насколько это возможно. Обратите внимание на ограничение по памяти.
*/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

int exit_flag = 0;

void* thread_func_scanf(void* args) {
    long long value = 0;

    while (scanf("%lld", &value) != EOF) {
        *(long long*) args += value;
        if (exit_flag == 1) {
            break;
        }
    }
    exit_flag = 1;

    return NULL;
}


int main(int argc, char** argv)
{
    long threads_count = strtol(argv[1], NULL, 10);
    long long* thread_sum = (long long*)malloc(threads_count * sizeof(long long));
    long long sum = 0;
    pthread_t thread[threads_count];
    int i = 0;

    for (i = 0; i < threads_count; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
        pthread_attr_setguardsize(&attr, 0);

        int errnum = pthread_create(&thread[i], &attr,
                                    (void*(*)(void *)) thread_func_scanf, thread_sum + i);
        if (errnum != 0) {
            fprintf(stderr, "thread create error\n");
            _exit(errnum);
        }
    }

    for (i = 0; i < threads_count; ++i) {
        int errnum = pthread_join(thread[i], NULL);
        if (errnum != 0) {
            fprintf(stderr, "thread join error\n");
            _exit(errnum);
        }
        sum += *(thread_sum + i);
    }

    free(thread_sum);

    printf("%lld\n", sum);
    return 0;
}
