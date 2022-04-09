/*
Программа при запуске сообщает на стандартный поток вывода свой PID, после чего читает со стандартного потока вывода целое число - начальное значение, которое затем будет изменяться.

При поступлении сигнала SIGUSR1 увеличить текущее значение на 1 и вывести его на стандартный поток вывода.

При поступлении сигнала SIGUSR2 - умножить текущее значение на -1 и вывести его на стандартный поток вывода.

При поступлении одного из сигналов SIGTERM или SIGINT необходимо завершить свою работу с кодом возврата 0.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.

Не забывайте выталкивать буфер вывода.
*/

#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t x = 0;
volatile sig_atomic_t sigusr1_flag = 0;
volatile sig_atomic_t sigusr2_flag = 0;
volatile sig_atomic_t return0_flag = 0;

void handler(int signum) {
    if (signum == SIGINT) {
        return0_flag = 1;
    } else if (signum == SIGTERM) {
        return0_flag = 1;
    } else if (signum == SIGUSR1) {
        sigusr1_flag = 1;
    } else if (signum == SIGUSR2) {
        sigusr2_flag = 1;
    }
}

int main() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    printf("%d\n", getpid());
    fflush(stdout);
    scanf("%d", &x);

    while(!return0_flag) {
        sched_yield();
        if (sigusr1_flag == 1) {
            x++;
            printf("%d\n", x);
            fflush(stdout);
            sigusr1_flag = 0;
        }
        if (sigusr2_flag == 1) {
            x *= -1;
            printf("%d\n", x);
            fflush(stdout);
            sigusr2_flag = 0;
        }
    }
    return 0;
}
