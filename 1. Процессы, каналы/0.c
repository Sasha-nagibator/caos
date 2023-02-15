/*
Реализуйте программу, которая принимает два аргумента: CMD - команда для запуска, IN - имя файла, направляемого на ввод.

Программа должна запускать указанную команду, передавая ей на вход содежимое файла IN.

На стандартный поток вывода вывести количество байт, которое было записано запущенной командой в стандартный поток вывода. Вывод самой команды никуда выводить не нужно.
*/

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char* cmd = argv[1];
    char* in_name = argv[2];

    int in = open(in_name, O_RDONLY);
    dup2(in, 0);
    close(in);

    int fds_pair[2];
    pipe(fds_pair);

    pid_t pid = fork();
    if (pid == 0) {
        dup2(fds_pair[1], 1);
        close(fds_pair[1]);
        if (execlp(cmd, cmd, NULL) == -1) {
            fprintf(stderr, "Exec error\n");
            _exit(1);
        }
    } else {
        close(fds_pair[1]);
        uint64_t total_count = 0;
        ssize_t count;
        char buffer[4096];
        while ((count = read(fds_pair[0], buffer, sizeof(buffer))) > 0) {
            total_count += count;
        }
        waitpid(pid, 0, 0);
        printf("%" PRIu64 "\n", total_count);
    }
}
