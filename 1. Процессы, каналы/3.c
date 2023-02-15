/*
Программе передаётся произвольное количество аргументов: CMD1, CMD2, ..., CMDN.

Необходимо реализовать эквивалент запуска их командной строки: CMD1 | CMD2 | ... | CMDN.

Родительский процесс должен завершаться самым последним!
*/

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main(int argc, char** argv)
{
    int real_args_count = argc - 1;
    int channels[real_args_count - 1][2];
    pipe(channels[0]);
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(channels[0][1], 1);
        close(channels[0][0]);
        close(channels[0][1]);
        execlp(argv[1], argv[1], NULL);
    }
    int i = 0;
    pid_t pid2;
    for (i = 1; i < real_args_count - 1; ++i) {
        pipe(channels[i]);
        pid2 = fork();
        if (pid2 == 0) {
            dup2(channels[i - 1][0], 0);
            dup2(channels[i][1], 1);
            for (int k = 0; k < i; ++k) {
                close(channels[k][1]);
                close(channels[k][0]);
            }
            execlp(argv[i + 1], argv[i + 1], NULL);
        }
    }
    pid_t pid3 = fork();
    if (pid3 == 0) {
        dup2(channels[real_args_count - 2][0], 0);
        for (int k = 0; k < real_args_count - 1; ++k) {
            close(channels[k][1]);
            close(channels[k][0]);
        }
        execlp(argv[real_args_count], argv[real_args_count], NULL);
    }
    for (int k = 0; k < real_args_count - 1; ++k) {
        close(channels[k][1]);
        close(channels[k][0]);
    }
    while (wait(NULL) > 0);
    return 0;
}
