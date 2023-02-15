/*
Программе передаётся два аргумента: CMD1 и CMD2. Необходимо запустить два процесса, выполняющих эти команды, и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.

В командной строке это эквивалентно CMD1 | CMD2.

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
    char* command1 = argv[1];
    char* command2 = argv[2];
    int pipe_fds[2];

    if (pipe(pipe_fds) == -1) {
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        return 1;
    }
    if (pid1 == 0) {
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
            return 1;
        }
        // output from command1 will be written in pipe instead of stdout
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        execlp(command1, command1, NULL);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        return 1;
    }
    if (pid2 == 0) {
        if (dup2(pipe_fds[0], STDIN_FILENO) == -1) {
            return 1;
        }
        // cmd1 output now is pipe's stdin
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        execlp(command2, command2, NULL);
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);

    while (wait(NULL) > 0);
}
