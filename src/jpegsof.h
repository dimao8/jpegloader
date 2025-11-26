/**
 * \file
 * \brief SOF extract module
 */

#ifndef JPEGSOF_H
#define JPEGSOF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct jpeg_context_tag jpeg_context_t;

/**
 * \brief Extract SOF
 * \param [in, out] context -- Previously initialized JPEG context
 * \param [in] n            -- SOF number. For example n = 3 means SOF3
 * \param [in] stream       -- Memory data stream
 * \param [in] stream_size  -- Size of the whole stream
 * \param [in, out] pos     -- Current position in the stream
 * \return Returns \c true if extraction was successful
 *
 * \note \c pos points to the byte after segment length. It will be moved
 * forward by ```segment_size```.
 *
 * \warning For v1.0 n can be only 0 (SOF0). Any other values will be marked as
 * unsupported.
 */
bool jpeg_sof_extract (jpeg_context_t *context, uint8_t n,
                       const uint8_t *stream, size_t stream_size, size_t *pos);

#endif // JPEGSOF_H