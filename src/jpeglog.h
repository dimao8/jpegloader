#ifndef JPEGDEBUG_H
#define JPEGDEBUG_H

#include <stdio.h>

#ifndef NDEBUG
#define DEBUG_LOG(...) \
  fprintf(stdout, __VA_ARGS__); \
  fflush(stdout)
#else
#define DEBUG_LOG(...)
#endif // NDEBUG

#define JPEG_LOG(...) \
  fprintf(stdout, __VA_ARGS__); \
  fflush(stdout)
#endif // JPEGDEBUG_H
