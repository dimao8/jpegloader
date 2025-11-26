#include "jpegcom.h"
#include "jpegcontext.h"
#include "jpegfunc.h"
#include "jpeglog.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* **************************** jpeg_com_extract *************************** */

bool
jpeg_com_extract (jpeg_context_t *context, const uint8_t *stream,
                  size_t stream_size, size_t *pos)
{
  if (context == NULL || stream == NULL || pos == NULL
      || (*pos >= stream_size))
    return false;

  if (*pos >= stream_size - 2)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[У] jpegloader: End of stream\n");
      return false;
    }

  size_t block_sz = ntohs (*(uint16_t *)(stream + *pos));
  if (block_sz > 2)
    {
      if (context->comment != NULL)
        {
          free (context->comment);
          context->comment = NULL;
        }

      context->comment = (char *)malloc (block_sz - 1);
      if (context->comment == NULL)
        {
          context->error = JPEG_ERROR_MALLOC;
          JPEG_LOG ("[E] jpegloader: Can not allocate memory\n");
          return false;
        }

      memcpy (context->comment, stream + *pos + 2, block_sz - 2);
      context->comment[block_sz - 1] = 0;
    }

  *pos += block_sz;
  return true;
}