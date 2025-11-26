/**
 * \file
 * \brief Miscellaneous functions
 */

#ifndef JPEGFUNC_H
#define JPEGFUNC_H

#include <stdint.h>

int clampi (int value, int min, int max);

/**
 * \brief Convert network byte order to host byte order fo 16-bit integers
 * \param [in] val -- 16-bit integer value
 * \return Function returns result of conversion
 */
uint16_t ntohs (uint16_t val);

/**
 * \brief Dezigzaging routine
 * \param [in] i -- Index in ZigZag order
 * \return Returns index in line-major order
 *
 * \warning The values less than 0 or greater than 63 will be converted to
 * zero.
 */
int dezigzaging (int i);

#endif // JPEGFUNC_H
