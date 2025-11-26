#include "jpegapp.h"
#include "jpegcontext.h"
#include "jpegfunc.h"
#include "jpeglog.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* **************************** jpeg_app_extract *************************** */

bool
jpeg_app_extract (jpeg_context_t *context, uint8_t n, const uint8_t *stream,
                  size_t stream_size, size_t *pos)
{
  if (context == NULL || stream == NULL || pos == NULL
      || (*pos >= stream_size))
    return false;

  if (n > 15)
    {
      context->error = JPEG_ERROR_NOT_SUPPORTED;
      DEBUG_LOG ("[D] jpegloader: APPn with n > 15 not supported\n");
      return false;
    }

  if (*pos >= stream_size - 2)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      DEBUG_LOG ("[D] jpegloader: End of stream\n");
      return false;
    }

  size_t block_sz = ntohs (*(uint16_t *)(stream + *pos));
  if (block_sz > 2)
    {
      if (context->app[n].data != NULL)
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: Duplicate APPn segments\n");
          return false;
        }

      context->app[n].data = (uint8_t *)malloc (block_sz - 2);
      if (context->app[n].data == NULL)
        {
          context->error = JPEG_ERROR_MALLOC;
          DEBUG_LOG ("[D] jpegloader: Can not allocate memory\n");
          return false;
        }

      context->app[n].size = block_sz - 2;
      memcpy (context->app[n].data, stream + *pos + 2, context->app[n].size);
    }

  *pos += block_sz;
  return true;
}