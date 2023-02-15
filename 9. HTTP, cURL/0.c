/* inf-IV-02-0
В аргументах командной строки передаются: 1) имя сервера; 2) путь к файлу на
сервере, начинающися с символа /.

Необходимо выполнить HTTP-GET запрос к серверу вывести содержимое файла на
стандартный поток вывода.

Запрещено использовать сторонние библиотеки. 
*/

#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


const size_t REQUEST_MAX_SIZE = 1024;
const size_t RESPONSE_MAX_SIZE = 1024;
const char CONTENT_LENGTH_STRING[] = "Content-Length: ";
const char CONTENT_DELIMETR[] = "\r\n\r\n";

const char SERVER_PORT[] = "80";

int main(int argc, char* argv[]) {
  char* server_hostname = argv[1];

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* result = NULL;

  int getaddrinfo_result = getaddrinfo(server_hostname, SERVER_PORT, &hints, &result);
  if (getaddrinfo_result != 0) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_result));

  int socket_fd = -1;
  struct addrinfo* result_it = NULL;
  for (result_it = result; result_it != NULL; result_it = result_it->ai_next) {

    socket_fd = socket(result_it->ai_family, result_it->ai_socktype, result_it->ai_protocol);
    if (socket_fd == -1) continue;


    int connect_result = connect(socket_fd, result_it->ai_addr, result_it->ai_addrlen);
    if (connect_result != -1) break;

    int close_result = close(socket_fd);
    if (close_result == -1) perror("close");

  }

  if (result_it == NULL) fprintf(stderr, "getaddrinfo: no suitable result\n");

  freeaddrinfo(result);

  char request[REQUEST_MAX_SIZE];
  char* server_filename = argv[2];
  snprintf(request, REQUEST_MAX_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", server_filename, server_hostname);

  ssize_t send_result = send(socket_fd, request, strlen(request), 0);
  if (send_result == -1) perror("send");

  char response[RESPONSE_MAX_SIZE];

  ssize_t recv_result = recv(socket_fd, response, RESPONSE_MAX_SIZE, 0);
  if (recv_result == -1) perror("recv");

  size_t content_length = 0;
  char* data_location = strstr(response, CONTENT_LENGTH_STRING) + strlen(CONTENT_LENGTH_STRING);
  sscanf(data_location, "%zu", &content_length);

  data_location = strstr(response, CONTENT_DELIMETR) + strlen(CONTENT_DELIMETR);
  ssize_t write_result = write(1, data_location, recv_result - (data_location - response));
  content_length -= write_result;

  while (content_length > 0) {
    recv_result = recv(socket_fd, response, RESPONSE_MAX_SIZE, 0);
    if (recv_result == -1) perror("recv");

    write_result = write(1, response, recv_result);
    content_length -= write_result;
  }

  int shutdown_result = shutdown(socket_fd, SHUT_RDWR);
  if (shutdown_result == -1) perror("shutdown");

  int close_result = close(socket_fd);
  if (close_result == -1) perror("close");

  return 0;
}
