/*
Программе передается два аргумента: имя файла с библиотекой и имя функции из
этой библиотеки.

Гарантируется, что функция имеет сигнатуру:
double function(double argument);

На стандартном потоке ввода подаются вещественные числа.
Необходимо применить к ним эту функцию, и вывести полученные значения.
Для однозначности вывода используйте формат %.3f.
*/

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

typedef double (*function_t)(double);

int main(int argc, char** argv) {
  double n = 0.;
  void* lib = dlopen(argv[1], RTLD_LAZY);
  if (lib == NULL) {
    fprintf(stderr, "dlopen error: %s\n", dlerror());
    _exit(1);
  }

  void* function = dlsym(lib, argv[2]);
  if (function == NULL) {
    fprintf(stderr, "dlsym error: %s\n", dlerror());
    _exit(2);
  }

  while (scanf("%lf", &n) != EOF) {
    printf("%.3f\n", ((function_t)function)(n));
  }
  dlclose(lib);
}
