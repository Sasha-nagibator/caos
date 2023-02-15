/*
Программе в качестве аргумента передается имя файла программы на языке Си. Необходимо попытаться её скомпилировать с помощью штатного компилятора gcc, после чего вывести на стандартный поток вывода: количество строк программы с ошибками (error), и количество строк программы с предупреждениями (warning). В одной строке может быть найдено несколько ошибок или предупреждений, - нужно вывести именно количество строк.

Запрещено создавать временные файлы для сохранения вывода ошибок компилятора. Используйте передачу текста от компилятора через каналы.
*/

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void find_problems(char* program, char* problem) {
    char* command1 = program;
    int pipe_fds1[2];
    int pipe_fds2[2];
    int pipe_fds3[2];
    int pipe_fds4[2];
    int status = 0;
    pipe(pipe_fds1);
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipe_fds1[1], 2);

        close(pipe_fds1[0]);
        close(pipe_fds1[1]);
        execlp("gcc", "gcc", command1, NULL);
    }

    pipe(pipe_fds2);
    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(pipe_fds1[0], 0);

        close(pipe_fds1[0]);
        close(pipe_fds1[1]);
        dup2(pipe_fds2[1], 1);
        close(pipe_fds2[1]);
        close(pipe_fds2[0]);
        execlp("grep", "grep", problem, NULL);
    }

    pipe(pipe_fds3);
    pid_t pid3 = fork();
    if (pid3 == 0) {
        dup2(pipe_fds2[0], 0);
        dup2(pipe_fds3[1], 1);
        close(pipe_fds1[0]);
        close(pipe_fds1[1]);
        close(pipe_fds2[0]);
        close(pipe_fds2[1]);
        close(pipe_fds3[1]);
        close(pipe_fds3[0]);
        execlp("grep", "grep", "-oh", "\\w*:\\w*:", NULL);
    }

    pipe(pipe_fds4);
    pid_t pid4 = fork();
    if (pid4 == 0) {
        dup2(pipe_fds3[0], 0);
        dup2(pipe_fds4[1], 1);
        close(pipe_fds1[0]);
        close(pipe_fds1[1]);
        close(pipe_fds2[0]);
        close(pipe_fds2[1]);
        close(pipe_fds3[1]);
        close(pipe_fds3[0]);
        close(pipe_fds4[1]);
        close(pipe_fds4[0]);
        execlp("uniq", "uniq", NULL);
    }

    pid_t pid5 = fork();
    if (pid5 == 0) {
        dup2(pipe_fds4[0], 0);
        close(pipe_fds1[0]);
        close(pipe_fds1[1]);
        close(pipe_fds2[0]);
        close(pipe_fds2[1]);
        close(pipe_fds3[1]);
        close(pipe_fds3[0]);
        close(pipe_fds4[1]);
        close(pipe_fds4[0]);
        execlp("wc", "wc", "-l", NULL);
    }

    close(pipe_fds1[0]);
    close(pipe_fds1[1]);
    close(pipe_fds2[0]);
    close(pipe_fds2[1]);
    close(pipe_fds3[0]);
    close(pipe_fds3[1]);
    close(pipe_fds4[0]);
    close(pipe_fds4[1]);

    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    waitpid(pid3, &status, 0);
    waitpid(pid4, &status, 0);
    waitpid(pid5, &status, 0);
}

int main(int argc, char** argv)
{
    find_problems(argv[1], "error");
    find_problems(argv[1], "warning");
}

