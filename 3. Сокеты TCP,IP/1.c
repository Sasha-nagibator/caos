/*
Задача int-III-03-1: posix/sockets/http-server-1

Необходимо реализовать программу-сервер, которой передаются два аргумента: номер порта и полный путь к каталогу с данными.

Программа должна прослушивать TCP-соединение на сервере localhost и указанным номером порта.

После получения сигнала SIGTERM или SIGINT сервер должен закончить обработку текущего соединения, если оно есть, после чего корректно завершить свою работу.

Внимание: в этой задаче признаком конца строк считается пара символов "\r\n", а не одиночный символ '\n'.

Каждое соединение должно обрабатываться следующим образом:

    Клиент отправляет строку вида GET ИМЯ_ФАЙЛА HTTP/1.1
    Клиент отправляет произвольное количество непустых строк
    Клиент отправляет пустую строку

После получения пустой строки от клиента, сервер должен отправить клиенту слеющие данные:

    Строку HTTP/1.1 200 OK, если файл существует, или HTTP/1.1 404 Not Found, если файл не существует, или HTTP/1.1 403 Forbidden, если файл существует, но не доступен для чтения
    Строку Content-Length: %d, где %d - размер файла в байтах
    Пустую строку
    Содержимое файла as-is

После отправки ответа клиенту, нужно закрыть соединение и не нужно ждать ожидать от клиента следующих запросов.
*/

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

volatile sig_atomic_t return0_flag = 0;
volatile sig_atomic_t server_fd;
volatile sig_atomic_t client_fd;

void exit_handler() {
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    return0_flag = 1;

}

int main(int argc, char** argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = exit_handler;
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    /* char* argv[3];
    argv[1] = "34568";
    argv[2] = "/home/aleksandr/Pictures/screenshots/2.txt"; */
    const char* port_str = argv[1];
    char* path = argv[2];
    if (path[strlen(path) - 1] != '/') {
        strcat(path, "/");
    }
    uint16_t port = htons(strtol(port_str, NULL, 10));

    server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - семейство протоколов для IPv4
    // SOCK_STREAM - тип в случае TCP. 0 - протокол нижележащего уровня
    // в переменной server - дескриптор сокета
    struct sockaddr_in adr; // sockaddr_in - структура чтобы задать адрес по протоколу IPv4

    adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    adr.sin_family = AF_INET; // задали семейство адресов
    adr.sin_port = port; // номер порта. htons меняет порядок байт от хоста к сети

    // в .sin_addr можно указать на каком адресе хотим слушать (если 0.0.0.0 то любой)
    bind(server_fd, (struct sockaddr *) &adr, sizeof adr); // bind - превязка сокета к адресу
    listen(server_fd, 128); // приступаем к прослушиванию входяших соединений. В очереди может ожидать макс
    // 128 клиентов

    while (!return0_flag) {

        client_fd = accept(server_fd, NULL, NULL); // принимаем клиента на сокете корорый
        if (client_fd == -1) {
            return 0;
        }
        // ранее открыли.
        ssize_t bytes_read;
        char buf[2048];
        bytes_read = read(client_fd, buf, 2048);  // читаем сообщение которое клиент послал.
        // Считываем от клиента в buf. Размер буфера - 2048

        char curr[256];
        int spaces = 0;
        for (int i = 0; i < bytes_read; ++i) {
            curr[i] = buf[i];
            if (buf[i] == ' ') {
                spaces++;
            }
            if (spaces == 2) {
                curr[i] = '\0';
                bytes_read = i;
                break;
            }
        }

        char filename[bytes_read - 4];
        strncpy(filename, curr + 4, bytes_read - 4);

        char realpath[strlen(path) + strlen(filename)];
        strcpy(realpath, path);
        strcat(realpath, filename);
        // puts(realpath);
        struct stat file_info;
        int errno = stat(realpath, &file_info);
        if (errno == -1) {
            write(client_fd, "HTTP/1.1 404 Not Found", sizeof("HTTP/1.1 404 Not Found"));
        } else if (access(realpath, R_OK) == -1) {
            write(client_fd, "HTTP/1.1 403 Forbidden", sizeof("HTTP/1.1 403 Forbidden"));
        } else {
            FILE* file = fopen(realpath, "r");

            int sz = (int) file_info.st_size;
            char buffer[sz];

            fread(buffer, 1, sz, file);

            fclose(file);

            dprintf(client_fd, "%s%d\r\n\r\n%s", "HTTP/1.1 200 OK\r\nContent-Length: ", sz, buffer);
        }
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);  // закрываем сокет работы с клиентом
    }

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd); // закрываем прослушивающий сокет
    return 0;
}
