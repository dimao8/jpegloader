#include "jpegfunc.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

/* ********************************* ntohs ********************************* */

uint16_t ntohs (uint16_t val)
{
#ifdef WORDS_BIGENDIAN
  return val;
#else
  return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
#endif // WORDS_BIGENDIAN
}