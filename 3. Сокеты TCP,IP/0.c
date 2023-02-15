/*
Задача int-III-03-0: posix/sockets/tcp-client

Программе передаются два аргумента: строка с IPv4-адресом в стандартной десятичной записи (четыре числа, разделенные точкой), и номер порта.

Программа должна установить соединение с указанным сервером, после чего читать со стандартного потока ввода целые знаковые числа в текстовом формате, и отправлять их в бинарном виде на сервер. Порядок байт - Little Endian.

В ответ на каждое полученное число, сервер отправляет целое число (в таком же формате), и все эти числа необходимо вывести на стандартный поток вывода в текстовом виде.

Если сервер по своей инициативе закроет соединение, то нужно завершить работу с кодом возврата 0.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    const char* ip_str = argv[1];

    const char* port_str = argv[2];
    uint16_t port = htons(strtol(port_str, NULL, 10));
    // htons меняет порядок байт от хоста к сети

    int fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - семейство протоколов для IPv4
    // SOCK_STREAM - тип в случае TCP. 0 - протокол нижележащего уровня
    // в переменной fd - дескриптор сокета
    struct sockaddr_in adr = {0}; // sockaddr_in - структура чтобы задать адрес по протоколу IPv4
    // все поля проинициализироваля нулями
    adr.sin_family = AF_INET; // задали семейство адресов
    adr.sin_port = port; // номер порта.
    inet_pton(AF_INET, ip_str, &adr.sin_addr); // в adr.sin_addr записываем ip адрес к которому
    // подключаемся
    connect(fd, (struct sockaddr *) &adr, sizeof adr); // указываем к кому подключаемся. Подключаемся через
    // сокет fd, указываем adr - адрес сервера

    int32_t number = 0;
    while (scanf("%d", &number) != EOF) {
        write(fd, &number, sizeof(number));

        int read_result = read(fd, &number, sizeof(number));

        if (!read_result) {
            break;
        } else {
            printf("%d\n", number);
        }
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 0;
}
