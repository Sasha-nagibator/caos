/*
Программа при запуске сообщает на стандартный поток вывода свой PID, выталкивает буфер вывода с помощью fflush, после чего начинает обрабатывать поступающие сигналы.

При поступлении сигнала SIGTERM необходимо вывести на стандартный поток вывода целое число: количество ранее поступивших сигналов SIGINT и завершить свою работу.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.
*/

#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t exit_flag = 0;

void handler(int signum) {
    if (signum == SIGINT) {
        counter++;
    } else if (signum == SIGTERM) {
        exit_flag = 1;
    }
}

int main() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    printf("%d\n", getpid());
    fflush(stdout);

    while(!exit_flag) {
        sched_yield();
    }
    printf("%d\n", counter);
}

/*
Строка 22 вопрос на сдачу зачем этот флаг
Строка 30 есть кое-что получше, что еще уменьшит потребление цпу, тоже советую узнать до сдачи
*/
