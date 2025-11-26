/**
 * \file
 * \brief Bit stream module
 *
 * This file contains jpeg_bitstream_t descriptor type and all function
 * prototypes for working with this type.
 *
 * Use bitstream_create() to create bit stream by capture existing byte array.
 * Function bitstream_create() returns pointer to the valid bit stream
 * descriptor of created bit stream or NULL pointer on error.
 */

#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Bit stream End of Stream value
 */
#define BITSTREAM_EOS UINT32_MAX

/**
 * \brief Bit stream descriptor
 * \struct jpeg_bitstream_t
 */
typedef struct jpeg_bitstream_tag jpeg_bitstream_t;

/**
 * \brief Create bit stream from byte array
 * \param [in] pointer -- Pointer to the byte array
 * \param [in] size    -- Size of the byte array
 * \return Returns the bit stream descriptor on success or NULL on fail.
 */
jpeg_bitstream_t *bitstream_create (uint8_t *pointer, size_t size);

/**
 * \brief Reset bit stream state
 * \param [in] stream -- Bit stream
 */
void bitstream_reset (jpeg_bitstream_t *stream);

/**
 * \brief Extract \c length bits from stream and move pointer forward
 * \param [in] stream -- Stream descriptor
 * \param [in] length -- Number of the bits to extract
 * \return Returns extracted data with righthand align (LSB)
 *
 * \warning Function bitstream_extract() can output 16 bit length words
 * maximum. If \c length is greater than 16 BITSTREAM_EOS will be returned.
 * \note If \c length is zero the returned value will be BITSTREAM_EOS.
 */
uint32_t bitstream_extract (jpeg_bitstream_t *stream, size_t length);

/**
 * \brief Extract single bit from stream and move pointer forward
 * \param [in] stream -- Stream descriptor
 * \return Returns bit value in LSB
 *
 * \note If bit stream is at the and BITSTREAM_EOS will be returned.
 */
uint32_t bitstream_next_bit (jpeg_bitstream_t *stream);

void bitstream_print_state (FILE* where, jpeg_bitstream_t *stream);

#endif // BITSTREAM_H
