/*
inf-IV-02-2 
В аргументе командной строки передается полный URL веб-страницы в формале URL
Необходимо загрузить эту страницу и вывести на стандартный поток вывода только заголовок
страницы, заключенный между тегами <title> и </title>

Используйте LibCURL. На сервер нужно отправить только исходный файл, который будет скомпилирован
и слинкован с нужными опциями
*/


#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char   *data;
    size_t length;
} buffer_t;

size_t callback_function(void *content, size_t size, size_t nmemb, void* user_data)
{
  buffer_t* buffer = user_data;
  size_t total_size = nmemb*size;
  buffer->data = realloc(buffer->data, buffer->length + total_size);
  memcpy(buffer->data + buffer->length, content, total_size);
  buffer->length += total_size;
  return total_size;
}

int main(int argc, char *argv[]) {
  const char* url = argv[1];
  CURL *curl = curl_easy_init();

  if (curl) {
    CURLcode res;

    // регистрация callback-функции записи
    int errnum = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);
    if (errnum != CURLE_OK) {
      fprintf(stderr, "%s\n", curl_easy_strerror(errnum));
      exit(EXIT_FAILURE);
    }

    // указатель &buffer будет передан в callback-функцию
    // параметром void *user_data
    buffer_t buffer;
    buffer.data = calloc(100*1024*1024, 1);
    buffer.length = 0;

    errnum = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    if (errnum != CURLE_OK) {
      fprintf(stderr, "%s\n", curl_easy_strerror(errnum));
      exit(EXIT_FAILURE);
    }

    errnum = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (errnum != CURLE_OK) {
      fprintf(stderr, "%s\n", curl_easy_strerror(errnum));
      exit(EXIT_FAILURE);
    }

    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    errbuf[0] = 0;

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      size_t len = strlen(errbuf);
      fprintf(stderr, "\nlibcurl: (%d) ", res);
      if(len)
        fprintf(stderr, "%s%s", errbuf,
                ((errbuf[len - 1] != '\n') ? "\n" : ""));
      else
        fprintf(stderr, "%s\n", curl_easy_strerror(res));
    }

    char output[2048];

    char *start = strstr(buffer.data, "<title>") + strlen("<title>");
    char *end = strstr(buffer.data, "</title>");
    strncpy(output, start, end - start);
    printf("%s", output);

    free(buffer.data);
    curl_easy_cleanup(curl);
  }
}
