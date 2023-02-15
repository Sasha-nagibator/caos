/* inf-IV-04-0
Реализуйте простую файловую систему, доступную только для чтения, которая
хранит файлы текстово-бинарном формате:

COUNT:INTEGER                   # количество файлов
FILENAME:STRING SIZE:INTEGER    # отдельная строка для каждого файла
# пустая строка (\n) - признак окончания текста и начала данных
BINARY DATA OF FILES IN ORDER AS LISTED ABOVE
Файловая система содержит только один корневой каталог, без подкаталогов.
Внутри файловой системы могут быть только регулярные файлы с правами 0444.

Для подготовки тестовых данных можно использовать утилиту cat.

Программа для реализации файловой системы должна поддерживать стандартный набор
опций FUSE и опцию для указания имени файла с образом файловой системы --src
ИМЯ_ФАЙЛА.

Используйте библиотеку FUSE версии 3.0 и выше.  На сервер нужно отпарвить
только исходный файл, который будет скомпилирован и слинкован с нужными опциями.

 To compile install libfuse3-dev and run gcc main.c -lfuse3
 To run: ./a.out --src src.txt ~/mount_dir
 To umount: umount -l ~/mount_dir
*/


#define FUSE_USE_VERSION 30
#define _GNU_SOURCE

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

static char* filesystem_source = NULL;


int fuse_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
                 struct fuse_file_info* fi, enum fuse_readdir_flags flags) {

  if (strcmp(path, "/") != 0) {
    return -ENOENT;
  }
  
  filler(out, ".", NULL, 0, 0);
  filler(out, "..", NULL, 0, 0);
  
  char* temp = filesystem_source;
  
  size_t file_count = 0;
  sscanf(temp, "%d\n", &file_count);
  
  temp = strchr(temp, '\n');
  ++temp;
  
  for (size_t i = 0; i < file_count; ++i) {
    char curr_path[PATH_MAX];
    off_t curr_size = 0;
    sscanf(temp, "%s %d\n", curr_path, &curr_size);
    
    temp = strchr(temp, '\n');
    ++temp;
    
    filler(out, curr_path, NULL, 0, 0);
  }

  return 0;
}

struct fileinfo {
    off_t file_size;
    int is_found;
    off_t data_offset;
    char* temp;
};

struct fileinfo read_in_loop (size_t file_count, char* temp, const char* path, int flag) {
  struct fileinfo fi;
  fi.is_found = 0;
  fi.data_offset = 0;
  for (size_t i = 0; i < file_count; ++i) {
    char curr_path[PATH_MAX];
    off_t curr_size = 0;
    sscanf(temp, "%s %d\n", curr_path, &curr_size);

    temp = strchr(temp, '\n');
    ++temp;

    if (strcmp(curr_path, path + 1) == 0) {
      fi.is_found = 1;
      fi.file_size = curr_size;
      if (flag == 1) {
        break;
      }
      if (flag == 2) {
        if (!fi.is_found) {
          fi.data_offset += curr_size;

        }
      }
    }
  }
  fi.temp = temp;
  return fi;
}

int fuse_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
  
  char* temp = filesystem_source;
  
  size_t file_count = 0;
  sscanf(temp, "%d", &file_count);
  
  if (strcmp(path, "/") == 0) {
    st->st_mode = 0555 | S_IFDIR;
    st->st_nlink = 2 + file_count;
    return 0;
  }
  temp = strchr(temp, '\n');
  ++temp;
  
  struct fileinfo fil = read_in_loop(file_count, temp, path, 1);

  
  if (!fil.is_found) {
    return -ENOENT;
  }
  
  st->st_mode = S_IFREG | 0444;
  st->st_nlink = 1;
  st->st_size = fil.file_size;

  return 0;
}

int fuse_read(const char* path, char* out, size_t size, off_t off, struct fuse_file_info* fi) {
  char* temp = filesystem_source;
  
  size_t file_count = 0;
  sscanf(temp, "%d\n", &file_count);
  
  temp = strchr(temp, '\n');
  ++temp;
  
  int is_found = 0;
  off_t file_size = 0;
  off_t data_offset = 0;
  //struct fileinfo fil = read_in_loop(file_count, temp, path, 2);

  for (size_t i = 0; i < file_count; ++i) {
    char curr_path[PATH_MAX];
    off_t curr_size = 0;
    sscanf(temp, "%s %d\n", curr_path, &curr_size);
    
    temp = strchr(temp, '\n');
    ++temp;
    
    if (strcmp(curr_path, path + 1) == 0) {
      is_found = 1;
      file_size = curr_size;
    }
    
    if (!is_found) {
      data_offset += curr_size;
    }
  }

  ++temp;
  
  if (off > file_size) {
    return 0;
  }
  
  if (off + size > file_size) {
    size = file_size - off;
  }
  
  const void* data = temp + data_offset + off;
  memcpy(out, data, size);

  return size;
}

int fuse_open(const char* path, struct fuse_file_info* fi) {
  char* temp = filesystem_source;
  
  size_t file_count = 0;
  sscanf(temp, "%d\n", &file_count);
  
  temp = strchr(temp, '\n');
  ++temp;

  struct fileinfo fil = read_in_loop(file_count, temp, path, 1);
  
  if (!fil.is_found) {
    return -ENOENT;
  }
  
  if (O_RDONLY != (fi->flags & O_ACCMODE)) {
    return -EACCES;
  }

  return 0;
}

struct fuse_operations callbacks = {
        .readdir = fuse_readdir,
        .getattr = fuse_stat,
        .open    = fuse_open,
        .read    = fuse_read,
};

typedef struct {
    char* src;
} fuse_options_t;

off_t get_file_size(int fd) {
  struct stat st;
  int fstat_result = fstat(fd, &st);
  if (fstat_result == -1) perror("fstat");

  off_t file_size = st.st_size;

  return file_size;
}

int main(int argc, char* argv[]) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  
  fuse_options_t fuse_options;
  memset(&fuse_options, 0, sizeof(fuse_options));
  
  struct fuse_opt opt_specs[] = {
          { "--src %s", offsetof(fuse_options_t, src), 0 },
          {NULL, 0, 0}    // end-of-array
  };
  
  fuse_opt_parse(&args, &fuse_options, opt_specs, NULL);
  
  off_t file_size = 0;
  if (fuse_options.src) {
    int filesystem_source_fd = open(fuse_options.src, O_RDONLY);
    if (filesystem_source_fd == -1) perror("open");

    file_size = get_file_size(filesystem_source_fd);

    filesystem_source = mmap(NULL, file_size,
                             PROT_READ, MAP_SHARED,
                             filesystem_source_fd, 0);

    if (filesystem_source == MAP_FAILED) perror("mmap");

    int close_result = close(filesystem_source_fd);
    if (close_result == -1) perror("close");
  }
  
  int fuse_main_result = fuse_main(args.argc, args.argv, &callbacks, NULL);
  
  int munmap_result = munmap(filesystem_source, file_size);
  if (munmap_result == -1) perror("munmap");

  return fuse_main_result;
}

