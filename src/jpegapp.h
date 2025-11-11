/**
 * \file
 * \brief APP extract module
 */

#ifndef JPEGAPP_H
#define JPEGAPP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct jpeg_context_tag jpeg_context_t;

/**
 * \brief Extract APP
 * \param [in, out] context -- Previously initialized JPEG context
 * \param [in] n            -- APP number. For example n = 3 means APP3
 * \param [in] stream       -- Memory data stream
 * \param [in] stream_size  -- Size of the whole stream
 * \param [in, out] pos     -- Current position in the stream
 * \return Returns \c true if extraction was successful
 *
 * \note \c pos points to the byte after segment length. It will be moved
 * forward by ```segment_size```.
 *
 * \note All APP segments will be skipped after all, but data will be placed
 * in the context.
 */
bool jpeg_app_extract (jpeg_context_t *context, uint8_t n,
                       const uint8_t *stream, size_t stream_size, size_t *pos);

#endif // JPEGAPP_H
