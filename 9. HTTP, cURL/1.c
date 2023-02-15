/* inf-IV-02-1
В аргументах командной строки передаются:
1) имя сервера;
2) путь к скрипту на сервере, начинающийся с символа /;
3) имя локального файла для отправки.

Необходимо выполнить HTTP-POST запрос к серверу, в котором отправить содержимое
файла.

На стандартный поток ввода вывести ответ сервера (исключая заголовки).

Запрещено использовать сторонние библиотеки.
*/


#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


int main(int argc, char *argv[]) {
  const char* server_hostname = argv[1];
  const char* server_filename = argv[2];
  const char* local_filename = argv[3];

  struct addrinfo* result;
  struct addrinfo* rp;
  struct addrinfo hints;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; // почему-то без этого была ошибка getaddrinfo: Bad value for ai_flags

  int errnum = getaddrinfo(server_hostname, "http", &hints, &result);
  if (errnum != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errnum));
    exit(EXIT_FAILURE);
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket error");
    exit(1);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    errnum = connect(server_fd, result->ai_addr, result->ai_addrlen);
    if (errnum != -1) {
      break;                      /* Success */
    }
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);           /* No longer needed */

  int fd = open(local_filename, O_RDONLY);
  if (fd == -1) {
    perror("file open error");
    exit(1);
  }

  struct stat st;
  if (fstat(fd, &st) == -1) {
    perror("stat error");
    exit(1);
  }
  int file_sz = (int)st.st_size;
  char* content = "";
  if (file_sz != 0) {
    content = mmap(NULL, file_sz, PROT_READ, MAP_PRIVATE, fd, 0);
  }

  int buf_size = file_sz + 4096;
  char* buf = malloc(buf_size * sizeof(char));
  snprintf(buf, buf_size,"POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: multipart/form-data\r\n"
                         "Connection: close\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",
           server_filename, server_hostname, file_sz, content);

  write(server_fd, buf, strnlen(buf, buf_size));
  FILE* socket_file = fdopen(server_fd, "r");
  char buffer[100 * 4096];
  fgets(buffer, sizeof(buffer), socket_file);
  while (strcmp(buffer, "\r\n") != 0) {
    fgets(buffer, sizeof(buffer), socket_file);
  }
  while (fgets(buffer, sizeof(buffer), socket_file)) {
    printf("%s", buffer);
  }
  munmap(content, file_sz);
  fclose(socket_file);
  close(server_fd);
  free(buf);
}

/* Строка 70 Я не понимаю, зачем в памяти держать весь файл
   Надо было использовать sendfile
 */
