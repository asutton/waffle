
#include <cxxabi.h>

#include <cstdio>

#include "debug.hpp"

std::string
demangle(const char* id) {
  char buf[2048];
  int result;
  std::size_t len = 2048;
  return __cxxabiv1::__cxa_demangle(id, buf, &len, &result);
}
