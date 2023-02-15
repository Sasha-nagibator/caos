/* inf-IV-03-0
Программе на стандартный поток ввода передается последовательность байт.

Необходимо вычислить контрольную сумму SHA-512 и вывести это значение в
hex-виде c префиксом 0x.

Используйте API OpenSSL/LibreSSL. Запуск сторонних команд через fork+exec
запрещен.

Отправляйте только исходный файл Си-программы с решением.
*/

#include <stdio.h>
#include <openssl/sha.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

int main() {
  SHA512_CTX context;
  SHA512_Init(&context);

  char buffer[SHA512_DIGEST_LENGTH];
  size_t bytes_read = read(STDIN_FILENO, buffer, SHA512_DIGEST_LENGTH);
  while (bytes_read > 0) {
    SHA512_Update(&context, buffer, bytes_read);
    bytes_read = read(STDIN_FILENO, buffer, SHA512_DIGEST_LENGTH);
  }

  char hash[SHA512_DIGEST_LENGTH];
  SHA512_Final(hash, &context);

  printf("0x");
  for (size_t i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
    printf("%02x", (uint8_t)hash[i]);
  }
  printf("\n");
}
