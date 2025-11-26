#include <jpegloader/jpegtypes.h>

#include "jpegcontext.h"
#include "jpegdqt.h"
#include "jpegfunc.h"
#include "jpeglog.h"

#include <stdio.h>
#include <string.h>

/* **************************** jpeg_dqt_extract *************************** */

bool
jpeg_dqt_extract (jpeg_context_t *context, const uint8_t *stream,
                  size_t stream_size, size_t *pos)
{
  if (context == NULL || stream == NULL || pos == NULL
      || (*pos >= stream_size))
    return false;

  if (*pos >= stream_size - 3)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: End of stream\n");
      return false;
    }

  size_t block_sz = ntohs (*(uint16_t *)(stream + *pos));
  uint8_t prec = DQT_PRECISION (*(stream + *pos + 2));
  uint8_t dest = DQT_DESTINATION (*(stream + *pos + 2));
  if (prec > 1)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Precision of DQT must be 0 (8 bit) or 1 (16 "
                "bit), (got%hhi)\n",
                prec);
      return false;
    }
  if (dest > 3)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Destination of DQT must be in range [0, 3] "
                "(got%hhi)\n",
                dest);
      return false;
    }

  if (DQT_DESTINATION (context->dqt[dest].prec_dest) != DQT_DESTINATION_INV)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Table %hhi is duplicated\n", dest);
      return false;
    }
  else
    {
      context->dqt[dest].prec_dest = *(stream + *pos + 2);
    }
  if (DQT_PRECISION (context->dqt[dest].prec_dest))
    {
      for (int i = 0; i < 64; i++)
        context->dqt[dest].table[i]
            = ntohs (((uint16_t *)(stream + *pos + 3))[i]);
    }
  else
    {
      for (int i = 0; i < 64; i++)
        context->dqt[dest].table[i] = (stream + *pos + 3)[i];
    }

  *pos += block_sz;
  return true;
}