/*
Задача inf-III-07-1: posix/sockets/icmp-ping

Программа принимает три аргумента: строку с IPv4-адресом, и два неотрицательных целых числа, первое из которых определяет общее время работы программы timeout, а второе - время между отдельными запросами в микросекундах interval.

Необходимо реализовать упрощённый аналог утилиты ping, которая определяет доступность удаленного хоста, используя протокол ICMP.

Программа должна последовательно отправлять echo-запросы к указанному адресу и подсчитывать количество успешных ответов. Между запросами, во избежание большой нагрузки на сеть, необходимо выдерживать паузу в interval микросекунд (для этого можно использовать функцию usleep).

Через timeout секунд необходимо завершить работу, и вывести на стандартный поток вывода количество полученных ICMP-ответов, соответствующих запросам.

В качестве аналога можно посмотреть утилиту /usr/bin/ping.

Указания: используйте инструменты ping и wireshark для того, чтобы исследовать формат запросов и ответов. Для того, чтобы выполняемый файл мог без прав администратора взаимодействовать с сетевым интерфейсом, нужно после компиляции установить ему capabilities командой: setcap cat_net_raw+eip PROGRAM. Контрольная сумма для ICMP-заголовков вычисляется по алгоритму из RFC-1071. 
*/

#include <netinet/ip_icmp.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>

const int ONE_IN_BIG_ENDIAN = 256;
const uint32_t PORT_NUM = 0;

static ushort checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
 *         beginning at location "addr".
 */
    register long sum = 0;

    while (count > 1)  {
        /*  This is the inner loop */
        sum += *(unsigned short*) addr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0) {
        sum += *(unsigned char *) addr;
    }

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~sum;
}

static ulong curent_time()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    ulong time_ms = time.tv_sec * 1000 + (time.tv_nsec / 1000000);
    return time_ms;
}

int main(int argc, char** argv)
{
    const char* ip_str = argv[1];
    uint32_t timeout = (uint32_t)strtol(argv[2], NULL, 10);
    uint32_t interval = (uint32_t)strtol(argv[3], NULL, 10);

    int server_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_ICMP);
    if (server_fd == -1) {
        _exit(1);
    }

    struct in_addr ip_addr;
    int inet_aton_return = inet_aton(ip_str, &ip_addr);
    if (inet_aton_return == 0) {
        _exit(2);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = PORT_NUM;
    addr.sin_addr = ip_addr;

    struct timeval time_check;
    time_check.tv_sec = 0;
    time_check.tv_usec = 123;

    uint32_t success_count = 0;
    ulong timer = 0;
    uint32_t seq_num = ONE_IN_BIG_ENDIAN;

    while (timer < timeout * 1000) {
        ulong start_time = curent_time();
        struct icmphdr icmp_packet = {0};
        icmp_packet.type = ICMP_ECHO;
        icmp_packet.un.echo.id = getpid();
        icmp_packet.un.echo.sequence = seq_num;
        icmp_packet.checksum = checksum(&icmp_packet, sizeof icmp_packet);

        sendto(server_fd, &icmp_packet, sizeof icmp_packet,
               0, (struct sockaddr*)&addr, sizeof addr);

        struct sockaddr_in recv_addr;
        int addr_len = sizeof recv_addr;

        ssize_t recv_res = recvfrom(server_fd,&icmp_packet,sizeof icmp_packet,0,
                              (struct sockaddr*)&recv_addr,&addr_len);

        if (recv_res > 0) {
            ++success_count;
        }
        seq_num += ONE_IN_BIG_ENDIAN;
        usleep(interval);
        timer += curent_time() - start_time;
    }

    printf("%d\n", success_count);
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    return 0;
}
