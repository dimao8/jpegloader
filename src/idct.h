/**
 * \file
 * \brief Inverse cosine transform
 */

#ifndef IDCT_H
#define IDCT_H

/**
 * \brief Inverse cosine transform
 * \param [out] output -- Values in 8x8 shape (line-major)
 * \param [in] input   -- DCT coefficients in 8x8 shape (line-major)
 */
void idct (int *output, const int *input);

#endif // IDCT_H
