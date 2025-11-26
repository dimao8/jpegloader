#include "jpegfunc.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

const uint8_t dzz[64]
    = { 0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,
        12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };

/* ********************************* clampi ******************************** */

int
clampi (int value, int min, int max)
{
  return (value < min) ? (min) : ((value > max) ? max : value);
}

/* ********************************* ntohs ********************************* */

uint16_t
ntohs (uint16_t val)
{
#ifdef WORDS_BIGENDIAN
  return val;
#else
  return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
#endif // WORDS_BIGENDIAN
}

/* ****************************** dezigzaging ****************************** */

int
dezigzaging (int i)
{
  if ((i < 0) || (i >= 64))
    return 0;
  else
    return dzz[i];
}
