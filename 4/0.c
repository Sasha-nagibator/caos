/*
Задача inf-III-04-0: highload/epoll-read-fds-vector

Реализуйте функцию с сигнатурой:

          extern size_t
          read_data_and_count(size_t N, int in[N])
        

которая читает данные из файловых дескрипторов in[X] для всех 0 ≤ X < N , и возвращает суммарное количество прочитанных байт из всех файловых дескрипторов.

Скорость операций ввода-вывода у файловых дескрипторов - случайная. Необходимо минимизировать суммарное астрономическое время чтения данных.

По окончании чтения необходимо закрыть все файловые дескрипторы.

Указание: используйте неблокирующий ввод-вывод. Для тестирования можно использовать socketpair.
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

extern size_t read_data_and_count(size_t N, int in[N])
{
    size_t total_read_count = 0;

    int epoll_fd = epoll_create(1); // создает очередь ядра, файловый дескриптор epoll_fd на нее ссылается
    // (параметр size игнорируется чтобы совместимость кода была)

    for (size_t i = 0; i < N; ++i) {
        int flags = fcntl(in[i], F_GETFL); // получили аттрибуты дискриптора
        flags |= O_NONBLOCK; // добавили к дискриптору флаг O_NONBLOCK
        fcntl(in[i], F_SETFL, flags);  // установили новые флаги

        struct epoll_event new_epoll_event;
        new_epoll_event.events = EPOLLIN;  // EPOLLIN - хотим для дискриптора
        // слушать готовность к чтению, т.е. если вызвать read то он выполнится без ожидания
        new_epoll_event.data.fd = in[i]; // метаданные - файловый дескриптор
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, in[i], &new_epoll_event);  // в interesting list epoll_fd
                // добавляем in[i]
    }
    size_t epoll_fds = N;
    struct epoll_event events[N];
    while (epoll_fds) {
        int new_events_count = epoll_wait(epoll_fd, events, epoll_fds, -1);
        // ядро заполнит массив events, макс кол-ао событий = epoll_fds. timeout = -1 значит что ждeм неограничено
        // epoll_wait возвращает число событий сколько он положил
        char temp_buffer[4096];
        for (size_t i = 0; i < new_events_count; ++i) {
            if (events[i].events & EPOLLIN) { // если на i-м событии произошло событие - готовность к чтению
                ssize_t read_result = 0;
                while ((read_result = read(events[i].data.fd, temp_buffer, 4096)) > 0) {
                    total_read_count += read_result; // читаем пока есть что читать
                }
                if (read_result == -1 && errno == EAGAIN) {
                    // при неблокирующем чтении не смогли больше прочитать
                    continue;
                } else if (read_result == 0) {  // конец файла
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                    // удаляем из наблюдения этот файловый дескриптор
                    close(events[i].data.fd); // закрываем его
                    --epoll_fds;
                }
            } else {
                fprintf(stderr, "Unhandled error\n");
            }
        }
    }
    return total_read_count;
}

/*
Вообще стоит пользоваться epoll_create1

Лучше добавлять EPOLLET, раз уж неблокирующий мод используется

Просто замечание
Will closing a file descriptor cause it  to  be  removed  from  all epoll interest lists?

Yes

*/
