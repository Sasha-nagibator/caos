#include <dlfcn.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <cstring>
#include "interfaces.h"

using namespace std;

struct ClassImpl
{
public:
    ClassImpl(void* lib_handler, void* constructor);
    ~ClassImpl();
    void* newInstanceWithSize(size_t sizeofClass);

private:
    void* (*constructor_)(void*);
    void* lib_handler_;
};

ClassImpl::ClassImpl(void* lib_handler, void* constructor)
{
  constructor_ = reinterpret_cast<void*(*)(void*)>(constructor);
  lib_handler_ = lib_handler;
}

ClassImpl::~ClassImpl()
{
  if (nullptr != lib_handler_)
    dlclose(lib_handler_);
}

void* ClassImpl::newInstanceWithSize(size_t size)
{
  void* pointer = (void*)calloc(size, sizeof(char));
  (*constructor_)(pointer);
  return pointer;
}

class ClassLoaderImpl
{
  public:
    ClassLoaderImpl();

    ClassLoaderError lastError() const;

    ClassImpl* loadClass(const string& fullyQualifiedName) {
      error_ = ClassLoaderError::NoError;
      string path = GetPath(fullyQualifiedName);
      auto access_res = access(path.data(), F_OK);
      if (access_res == -1) {
        error_ = ClassLoaderError::FileNotFound;
        return nullptr;
      }
      auto handle = dlopen(path.data(), RTLD_NOW);
      if (nullptr == handle) {
        error_ = ClassLoaderError::LibraryLoadError;
        return nullptr;
      }
      auto constructor = dlsym(handle, GetNameAfterMangling(fullyQualifiedName).data());
      if (nullptr == constructor) {
        error_ = ClassLoaderError::NoClassInLibrary;
        dlclose(handle);
        return nullptr;
      }
      return new ClassImpl(handle, constructor);
    }

  private:
    static string GetPath(const string& fullyQualifiedName);
    static string GetNameAfterMangling(const string& fullyQualifiedName);
    ClassLoaderError error_;
};

ClassLoaderImpl::ClassLoaderImpl() { error_ = ClassLoaderError::NoError; }

ClassLoaderError ClassLoaderImpl::lastError() const { return error_; }

string ClassLoaderImpl::GetPath(const string &fullyQualifiedName) {
  string path(getenv("CLASSPATH"));
  path += "/";
  for (auto i : fullyQualifiedName) {
    path += fullyQualifiedName[i];
    if (path[path.length() - 1] == ':') {
      path[path.length() - 1] = '/';
    }
  }
  return path += ".so";
}

string ClassLoaderImpl::GetNameAfterMangling(const string &fullyQualifiedName) {
  /* name = _ZN + str(Class name length) + Class name + C1Ev */
  string name_after_mangling("_ZN");
  const char* start = fullyQualifiedName.data();
  const char* end = nullptr;
  while (true) {
    end = strchr(start, ':');
    if (nullptr == end) {
      name_after_mangling += to_string(strlen(start));
      name_after_mangling += string(start);
      break;
    }
    name_after_mangling += to_string(end - start);
    name_after_mangling += string(start, end);
    start = end + 2;
  }
  name_after_mangling += "C1Ev"; // C1 - reserved name for constructor
  // E means this, v is varible type
  return name_after_mangling;
}

ClassLoader::ClassLoader() { pImpl = new ClassLoaderImpl; }

AbstractClass::AbstractClass() {}

AbstractClass::~AbstractClass() { delete pImpl; }

void* AbstractClass::newInstanceWithSize(size_t sizeofClass)
{
  return pImpl->newInstanceWithSize(sizeofClass);
}

AbstractClass* ClassLoader::loadClass(const string &fullyQualifiedName)
{
  auto* abs_cls = new AbstractClass;
  abs_cls->pImpl = pImpl->loadClass(fullyQualifiedName);
  return abs_cls;
}

ClassLoaderError ClassLoader::lastError() const { return pImpl->lastError(); }

ClassLoader::~ClassLoader() { delete pImpl; }

