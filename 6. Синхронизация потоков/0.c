/*
Задача inf-III-06-0: posix/threads/mutex

Программа запускается с двумя целочисленными аргументами: N>0 - количество итераций; и k>0 - количество потоков.

Необходимо создать массив из k вещественных чисел, после чего запустить k потоков, каждый из которых работает со своим элементом массива и двумя соседними.

Каждый поток N раз увеличивает значение своего элемента на 1, увеличивает значение соседа слева на 0.99, и увеличивает значение соседа справа на 1.01.

Для потоков, у которых нет соседей справа (k-1) или слева (0), соседними считать первое и последнее значение массива соответственно.

После того, как все потоки проведут N итераций, необходимо вывести значения всех элементов.

Запрещено использовать глобальные переменные.

Для вывода используйте формат %.10g.
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

struct task {
    long long k;
    long long N;
    long long i;
    pthread_mutex_t* mutex[3];
    double* arr;
};

void* add(void* args) {
    struct task arg = *(struct task*)args;
    int iter = 0;
    for (iter = 0; iter < arg.N; ++iter) {

        pthread_mutex_lock(arg.mutex[0]);
        arg.arr[(arg.i - 1 + arg.k) % arg.k] += 0.99;
        pthread_mutex_unlock(arg.mutex[0]);

        pthread_mutex_lock(arg.mutex[1]);
        arg.arr[arg.i] += 1;
        pthread_mutex_unlock(arg.mutex[1]);

        pthread_mutex_lock(arg.mutex[2]);
        arg.arr[(arg.i + 1) % arg.k] += 1.01;
        pthread_mutex_unlock(arg.mutex[2]);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    long long N = (long long)strtol(argv[1], NULL, 10);
    long long k = (long long)strtol(argv[2], NULL, 10);
    pthread_t threads[k];
    pthread_mutex_t lock[k];
    double arr[k];
    int i = 0;
    for(i = 0; i < k; ++i) {
        arr[i] = 0;
        pthread_mutex_init(&lock[i], NULL);
    }
    struct task args[k];
    for(i = 0; i < k; ++i) {
        args[i].k = k;
        args[i].N = N;
        args[i].i = i;
        args[i].mutex[0] = &lock[(i - 1 + k) % k];
        args[i].mutex[1] = &lock[i];
        args[i].mutex[2] = &lock[(i + 1) % k];
        args[i].arr = (double*)arr;
        pthread_create(&threads[i], NULL, &add, (void*)&args[i]);
    }
    for(i = 0; i < k; ++i) {
        pthread_join(threads[i], NULL);
    }
    for(i = 0; i < k; ++i) {
        pthread_mutex_destroy(&lock[i]);
        printf("%.10g ", arr[i]);
    }
    return 0;
}
