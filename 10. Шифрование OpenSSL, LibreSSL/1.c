/* inf-IV-03-1
Программе передается аргумент - пароль.

На стандартый поток ввода подаются данные, зашифрованные алгоритмом AES-256-CBC
с солью. Для получения начального вектора и ключа из пароля и соли используется
алгоритм SHA-256.

Необходимо расшифровать данные и вывести их на стандартый поток вывода.

Используйте API OpenSSL/LibreSSL. Запуск сторонних команд через fork+exec
запрещен.

Отправляйте только исходный файл Си-программы с решением.



To compile use gcc main.c -lcrypto.
   To test you can save encrypted text in a
   file and read from file instead of stdin */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>
#include <fcntl.h>

const int BITS_IN_BYTE = 8;
const int KEY_SIZE_IN_BITS = 256;
const int IV_SIZE_IN_BITS = 128;
const int SALT_SIZE = 8;

int main(int argc, char** argv) {
  const char* password = argv[1];
  char key[KEY_SIZE_IN_BITS / BITS_IN_BYTE];
  char iv[IV_SIZE_IN_BITS / BITS_IN_BYTE];
  char salted__[SALT_SIZE];
  int fd = STDIN_FILENO;
  // int fd = open("encrypted.enc", O_RDONLY);
  read(fd, salted__, SALT_SIZE);
  if (strncmp(salted__, "Salted__", 8) != 0) {
    perror("Message is not encrypted correctly");
    exit(1);
  }
  char salt[SALT_SIZE];
  read(fd, salt, SALT_SIZE);

  // Создание контекста
  EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();

  // Генерация ключа и начального вектора из
  // пароля произвольной длины и 8-байтной соли
  EVP_BytesToKey(
          EVP_aes_256_cbc(),    // алгоритм шифрования
          EVP_sha256(),         // алгоритм хеширования пароля
          salt,                 // соль
          password, strlen(password), // пароль
          1,                    // количество итераций хеширования
          key,          // результат: ключ нужной длины
          iv            // результат: начальный вектор нужной длины
  );

  // Начальная стадия: инициализация
  EVP_DecryptInit(
          context,                  // контекст для хранения состояния
          EVP_aes_256_cbc(),    // алгоритм шифрования
          key,                  // ключ нужного размера
          iv                    // начальное значение нужного размера
  );
  int block_size = EVP_CIPHER_CTX_block_size(context);
  char input[block_size];
  char output[block_size];
  int read_result = read(fd, input, block_size);
  int bytes_to_write = 0;
  while (read_result > 0) {
    if (!EVP_DecryptUpdate(context, output, &bytes_to_write, input, read_result)) {
      perror("EVP_DecryptUpdate error");
      exit(2);
    }
    write(STDOUT_FILENO, output, bytes_to_write);
    read_result = read(fd, input, block_size);
  }
  if (!EVP_DecryptFinal(context, output, &bytes_to_write)) {
    perror("EVP_DecryptFinal error");
    exit(3);
  }
  write(STDOUT_FILENO, output, bytes_to_write);
  EVP_CIPHER_CTX_free(context);
}
