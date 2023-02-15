/*
Задача inf-III-07-0: posix/sockets/udp-client

Аргументом программы является целое число - номер порта на сервере localhost.

Программа читает со стандартного потока ввода целые числа в тектовом формате, и отправляет их в бинарном виде (little-endian) на сервер как UDP-сообщение.

В ответ сервер отправляет целое число (также в бинарном виде, little-endian), которое необходимо вывести на стандартный поток вывода.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    const char* ip_str = "127.0.0.1";

    const char* port_str = argv[1];
    uint16_t port = htons(strtol(port_str, NULL, 10));
    // htons меняет порядок байт от хоста к сети

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        _exit(1);
    }

    struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr = inet_addr(ip_str),
            .sin_port = port
    };

    int32_t number = 0;
    while (scanf("%d", &number) != EOF) {
        ssize_t errnum = sendto(fd, &number, sizeof number, 0, (const struct sockaddr*)&addr, sizeof addr);
        if (errnum == -1) {
            _exit(2);
        }
        ssize_t read_result = recvfrom(fd, &number, sizeof number, 0, NULL, NULL);
        if (read_result == -1) {
            _exit(3);
        } else if (!read_result) {
            break;
        } else {
            printf("%d\n", number);
        }
    }

    close(fd);
    return 0;
}
