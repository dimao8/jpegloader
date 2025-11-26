#ifndef JPEGDQT_H
#define JPEGDQT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct jpeg_context_tag jpeg_context_t;

/**
 * \brief Extract DQT
 * \param [in, out] context -- Previously initialized JPEG context
 * \param [in] stream       -- Memory data stream
 * \param [in] stream_size  -- Size of the whole stream
 * \param [in, out] pos     -- Current position in the stream
 * \return Returns \c true if extraction was successful
 *
 * \note \c pos points to the byte after segment length. It will be moved
 * forward by ```segment_size```.
 */
bool jpeg_dqt_extract (jpeg_context_t *context, const uint8_t *stream,
                       size_t stream_size, size_t *pos);

#endif // JPEGDQT_H