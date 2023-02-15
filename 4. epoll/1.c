/*
Задача inf-III-04-1: highload/epoll-read-write-socket

Программе задается единственный аргумент - номер TCP-порта.

Необходимо принимать входящие соединения на TCP/IPv4 для сервера localhost, читать данные от клиентов в текстовом виде, и отправлять их обратно в текстовом виде, заменяя все строчные буквы на заглавные. Все обрабатываемые символы - из кодировки ASCII.

Одновременных подключений может быть много. Использовать несколько потоков или процессов запрещено.

Сервер должен корректно завершать работу при получении сигнала SIGTERM.

Указание: используйте неблокирующий ввод-вывод.
*/

#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>

volatile sig_atomic_t return0_flag = 0;


void exit_handler() {
    return0_flag = 1;
}


int main(int argc, char** argv) {
    const int useless = 123;

    int epoll_fd = epoll_create(useless);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = exit_handler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &action, NULL);

    const char* port_str = argv[1];

    uint16_t port = htons(strtol(port_str, NULL, 10));

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in adr;

    adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    adr.sin_family = AF_INET;
    adr.sin_port = port;

    bind(server_fd, (struct sockaddr *) &adr, sizeof(adr));

    int flags = fcntl(server_fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(server_fd, F_SETFL, flags);

    struct epoll_event server_events;
    server_events.events = EPOLLIN | EPOLLOUT | EPOLLET;

    server_events.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &server_events);

    listen(server_fd, 128);
    const int ready_list_cap = 12345;
    struct epoll_event ready_list[ready_list_cap];

    // printf("%d", getpid());
    // fflush(stdout);

    while (!return0_flag) {
        int ready_count = epoll_wait(epoll_fd, ready_list, ready_list_cap, -1);
        if (return0_flag) {
            continue;
        }
        int i;
        for (i = 0; i < ready_count; ++i) {
            if (ready_list[i].data.fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd == -1) {
                    continue;
                }
                int flags = fcntl(client_fd, F_GETFL);
                flags |= O_NONBLOCK;
                fcntl(client_fd, F_SETFL, flags);

                struct epoll_event new_client_events;
                new_client_events.events = EPOLLIN | EPOLLOUT | EPOLLET;

                new_client_events.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &new_client_events);
            }

            else {
                char message[2048];
                int message_len;
                while ((message_len = read(ready_list[i].data.fd, message, 2048)) > 0) {
                    int k;
                    for (k = 0; k < message_len; ++k) {
                        message[k] = toupper(message[k]);
                    }
                    write(ready_list[i].data.fd, message, message_len);
                }
                if (message_len == 0) {  // конец файла
                    shutdown(ready_list[i].data.fd, SHUT_RDWR);
                    close(ready_list[i].data.fd);
                }
            }
        }
    }

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    return 0;

}
