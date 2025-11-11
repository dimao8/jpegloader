/**
 * \file
 * \brief Miscellaneous function
 */

#ifndef JPEGFUNC_H
#define JPEGFUNC_H

#include <stdint.h>

/**
 * \brief Convert network byte order to host byte order fo 16-bit integers
 * \param [in] val -- 16-bit integer value
 * \return Function returns result of conversion
 */
uint16_t ntohs (uint16_t val);

#endif // JPEGFUNC_H
