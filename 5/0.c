/*
Задача inf-III-05-0: posix/threads/simple-create-join

На стандартном потоке ввода задается последовательность целых чисел.

Необходимо прочитать эти числа, и вывести их в обратном порядке.

Каждый поток может прочитать или вывести только одно число.

Используйте многопоточность, запуск процессов запрещен.
*/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* thread_func(void* args) {
    long long value;
    if (scanf("%lld", &value) != EOF) {
        pthread_t new_thread;
        int errnum = pthread_create(&new_thread, NULL, thread_func, NULL);
        if (errnum != 0) {
            fprintf(stderr, "thread create error\n");
            _exit(errnum);
        }
        errnum = pthread_join(new_thread, NULL);
        if (errnum != 0) {
            fprintf(stderr, "thread join error\n");
            _exit(errnum);
        }
        printf("%lld\n", value);
    }
    return NULL;
}

int main()
{
    pthread_t thread;

    int errnum = pthread_create(&thread, NULL, thread_func, NULL);
    if (errnum != 0) {
        fprintf(stderr, "thread create error\n");
        _exit(errnum);
    }

    errnum = pthread_join(thread, NULL);
    if (errnum != 0) {
        fprintf(stderr, "thread join error\n");
        _exit(errnum);
    }
    return 0;
}
