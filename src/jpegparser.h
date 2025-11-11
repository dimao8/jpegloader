#ifndef JPEGPARSER_H
#define JPEGPARSER_H

#include "jpegloader/jpegtypes.h"

typedef struct jpeg_context_tag jpeg_context_t;
typedef enum jpeg_error_tag jpeg_error_t;

/**
 * \brief Create empty jpeg context
 * \return Returns empty jpeg context
 *
 * \brief Create jpeg_context
 */
jpeg_context_t *jpeg_init ();

/**
 * \brief Release jpeg context
 * \param [in] jpeg_context -- Context of the JPEG image
 *
 * Call this function to free any dynamically allocated data. Th jpeg_free
 * function is safe with NULL as jpeg_context argument.
 */
void jpeg_free_context (jpeg_context_t *jpeg_context);

/**
 * \brief Decode jpeg from context data
 * \param [in] jpeg_context -- JPEG context
 * \param [out] header      -- Output JPEG header
 * \param [out] data        -- Pointer to the data array
 * \return Returns result of the decoding (error code)
 */
jpeg_error_t jpeg_decoding (jpeg_context_t *jpeg_context,
                            jpeg_header_t *header, void **data);

#endif // JPEGPARSER_H
