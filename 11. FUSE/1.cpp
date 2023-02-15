/*  	inf-IV-04-1
Реализуйте файловую систему, доступную только для чтения, которая строит
объединение нескольких каталогов в одно дерево, по аналогии с UnionFS, по
следующим правилам:

- если встречаются два файла с одинаковым именем, то правильным считается тот, у
которого более поздняя дата модификации;
- если встречаются два каталога с одинаковым именем, то нужно создавать
объединение их содержимого.  Внутри файловой системы могут быть только
регулярные файлы с правами не выше 0444 и подкаталоги с правами не выше 0555.

Программа для реализации файловой системы должна поддерживать стандартный набор
опций FUSE и опцию для указания списка каталогов для объединения
--src КАТАЛОГ_1:КАТАЛОГ_2:...:КАТАЛОГ_N.

Используйте библиотеку FUSE версии 3.0 и выше.  На сервер нужно отправить
только исходный файл, который будет скомпилирован и слинкован с нужными опциями.
*/

/*
  g++ main.cpp -lfuse3
  ./a.out --src a:b:e ~/test
  umount -l ~/test
 */

#define FUSE_USE_VERSION 30
#include <fuse3/fuse.h>
#include <sys/mman.h>
#include <ftw.h>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

using namespace std::string_literals;

using T = std::vector<std::string>;

T parse(std::string src, char delim) {
  T result;
  size_t pos = 0;
  size_t oldpos = 0;

  if (src.back() != delim) {
    src.push_back(delim);
  }
  for (int i = 0; i < src.length(); ++i) {
    if (src[i] == delim) {
      pos = i;
    }
    auto c = src.substr(oldpos, pos - oldpos);
    if (!c.empty()) {
      if (oldpos == 0 && src[0] != delim) {
        result.push_back(src.substr(oldpos, pos - oldpos));
      } else {
        result.push_back(src.substr(oldpos + 1, pos - oldpos - 1));
      }
    }
    oldpos = pos;
  }
  return result;
}

timespec time_last_modified(const std::string& path) {
  struct stat st {};
  stat(path.c_str(), &st);
  return st.st_mtim;
}

struct File {
  [[nodiscard]] const File* translate_path(int i, const T& available_paths) const;
  void get_best_path(int i, T& available_paths, const std::string& real_path);
  std::map<std::string, File> files;
  std::string path;
};

void File::get_best_path(int i, T& available_paths, const std::string& real_path) {
  if (i >= available_paths.size()) {
    return;
  }
  std::string curr = available_paths[i];

  if (files.find(curr) == files.end()) {
    files[curr] = File{};
    files[curr].path = real_path;
  } else {
    if (i == available_paths.size() - 1) {
      timespec a = time_last_modified(files[curr].path);
      timespec b = time_last_modified(real_path);
      if (a.tv_sec < b.tv_sec) {
        files[curr].path = real_path;
      }
    }
  }
  files[curr].get_best_path(i + 1, available_paths, real_path);
}

const File* File::translate_path(int i, const T& available_paths) const {
  auto it = files.find(available_paths[i]);
  if (i == available_paths.size() - 1) {
    return &it->second;
  }
  if (it == files.end()) {
    return nullptr;
  }
  return it->second.translate_path(i + 1, available_paths);
}

File root;

int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info* fi,
                 enum fuse_readdir_flags flags) {
  filler(buf, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
  filler(buf, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
  File* f;

  const File* file =
          static_cast<std::string>(path) == "/"
          ? &root
          : root.translate_path(
                0, parse(static_cast<std::string>(path).substr(1), '/'));

  if (!file) {
    return -ENOENT;
  }

  for (auto& it : file->files) {
    filler(buf, it.first.c_str(), nullptr, 0,
           static_cast<fuse_fill_dir_flags>(0));
  }
  return 0;
}

int read_callback(const char* path, char* buf, size_t size, off_t offset,
                  struct fuse_file_info* fi) {
  if (static_cast<std::string>(path) == "/") {
    return -EISDIR;
  }

  const File* file = root.translate_path(
          0, parse(static_cast<std::string>(path).substr(1), '/'));

  if (!file) {
    return -ENOENT;
  }

  int fd = open(file->path.c_str(), O_RDONLY);
  ssize_t read_result = read(fd, buf, size);
  close(fd);

  return static_cast<int>(read_result);
}

int getattr_callback(const char* path, struct stat* st,
                     struct fuse_file_info* fi) {
  if (static_cast<std::string>(path) == "/") {
    st->st_mode = S_IFDIR | 0555;
    return 0;
  }

  const File* file = root.translate_path(
          0, parse(static_cast<std::string>(path).substr(1), '/'));

  auto status = stat(file->path.c_str(), st);

  if (S_ISDIR(st->st_mode)) {
    st->st_mode = S_IFDIR | 0555;
  } else {
    st->st_mode = S_IFREG | 0444;
  }
  return status;
}

static int offset;

int tree_callback(const char* dirpath,
                  int (*fn)(const char* fpath, const struct stat* sb,
                            int typeflag, struct FTW* ftwbuf),
                  int nopenfd, int flags) {
  T available_paths = parse(dirpath + 1, '/');
  root.get_best_path(offset, available_paths,
                             std::string(dirpath, strlen(dirpath)));
  return 0;
}

struct fuse_operations callbacks = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

struct fuse_options_t {
  char* src;
};

struct fuse_opt opt_specs[] = {{"--src %s", offsetof(fuse_options_t, src), 0},
                                 {nullptr, 0, 0}};

int main(int argc, char* argv[]) {
  fuse_options_t fuse_options{};
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  int fuse_opt_parse_result = fuse_opt_parse(&args, &fuse_options, opt_specs, nullptr);
  if (fuse_opt_parse_result == -1) {
    perror("fuse_opt_parse error");
    exit(1);
  }
  T directories = parse(fuse_options.src, ':');
  for (auto& name : directories) {
    char* path = realpath(name.c_str(), nullptr);
    name = std::string(path, strlen(path));
    offset = static_cast<int>(parse(name.substr(1), '/').size());
    nftw(name.c_str(), reinterpret_cast<__nftw_func_t>(tree_callback), 0, 0);
  }

  int fuse_result = fuse_main(args.argc, args.argv, &callbacks, nullptr);
  fuse_opt_free_args(&args);
  return fuse_result;
}
