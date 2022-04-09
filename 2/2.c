/*
Программе в качестве аргументов передаются N имен текстовых файлов.

Программа должна обрабатывать множество сигналов от SIGRTMIN до SIGRTMAX, причем номер сигнала в диапазоне от SIGRTMIN+1 определяет порядковый номер файла из аргументов:

x = signo - SIGRTMIN; // SIGRTMIN <= signo <= SIGRTMAX
                      // 1 <= x <= SIGRTMAX-SIGRTMIN

При получении очередного сигнала необходимо прочитать одну строку из определенного файла и вывести её на стандартный поток вывода.

При получении сигнала с номером SIGRTMIN, т.е. при номере аргумента, равным 0, - корректно завершить свою работу с кодом 0.

Все остальные сигналы нужно игнорировать.

Если для вывода используются высокоуровневые функции стандартной библиотеки Си, то необходимо выталкивать буфер обмена после вывода каждой строки.
*/

#include <sys/signalfd.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void read_and_write(FILE* file) {
    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), file);
    fputs(buffer, stdout);
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    int i = 0;
    int real_args_count = argc - 1;
    FILE* files[argc - 1];
    for (i = 0; i < real_args_count; ++i) {
        files[i] = fopen(argv[i + 1], "r");
    }
    sigset_t all_signals_mask;
    sigset_t rt_signals_mask;
    sigemptyset(&rt_signals_mask);
    sigfillset(&all_signals_mask);
    sigprocmask(SIG_BLOCK, &all_signals_mask, NULL);
    for (i = 0; i <= real_args_count; ++i) {
        sigaddset(&rt_signals_mask, SIGRTMIN + i);
    }
    // printf("%d\n", getpid());
    // fflush(stdout);
    int fd = signalfd(-1, &rt_signals_mask, 0);
    struct signalfd_siginfo siginfo_struct;
    while (siginfo_struct.ssi_signo != SIGRTMIN) {
        read(fd, &siginfo_struct, sizeof(siginfo_struct));
        if (siginfo_struct.ssi_signo != SIGRTMIN) {
            read_and_write(files[siginfo_struct.ssi_signo - SIGRTMIN - 1]);
        }
    }
    for (i = 0; i < real_args_count; ++i) {
        fclose(files[i]);
    }
    close(fd);
    return 0;
}

/*
Строка 26 правильнее игнорировать все сигналы, а не блокировать вообще говоря. Для другого сделана эта механика с блокировкой
*/
