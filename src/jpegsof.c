#include <jpegloader/jpegtypes.h>

#include "jpegsof.h"
#include "jpegcontext.h"
#include "jpeglog.h"
#include "jpegfunc.h"

#include <stdio.h>
#include <stdlib.h>

/* **************************** jpeg_sof_extract *************************** */

bool
jpeg_sof_extract (jpeg_context_t *context, uint8_t n, const uint8_t *stream,
                  size_t stream_size, size_t *pos)
{
  if (context == NULL || stream == NULL || pos == NULL
      || (*pos >= stream_size))
    return false;

  if (*pos >= stream_size - 8)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      DEBUG_LOG ("[D] jpegloader: End of stream\n");
      return false;
    }

  size_t block_sz = ntohs (*(uint16_t *)(stream + *pos));
  context->header.header_n = n;
  context->header.precision = stream[*pos + 2];
  context->header.height = ntohs (*(uint16_t *)(stream + *pos + 3));
  context->header.width = ntohs (*(uint16_t *)(stream + *pos + 5));
  context->header.n_component = stream[*pos + 7];
  if (*pos >= stream_size - 8 - 3 * context->header.n_component)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      DEBUG_LOG ("[D] jpegloader: End of stream\n");
      return false;
    }

  // Check for static part
  if (context->header.width == 0)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      DEBUG_LOG ("[D] jpegloader: SOFn.X can not be zero\n");
      return false;
    }

  switch (n)
    {
    case 0: // Baseline DCT
      if (context->header.precision != 8)
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF0.P must be 8 (got %hhi)\n",
                     context->header.precision);
          return false;
        }
      if (context->header.n_component < 1)
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF0.Nf can not be zero\n");
          return false;
        }
      break;

    case 1: // Extended sequential DCT
    case 9:
      if ((context->header.precision != 8)
          && (context->header.precision != 12))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF1.P must be 8 ar 12 (got %hhi)\n",
                     context->header.precision);
          return false;
        }
      if (context->header.n_component < 1)
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF1.Nf can not be zero\n");
          return false;
        }
      break;

    case 2: // Progressive DCT
    case 10:
      if ((context->header.precision != 8)
          && (context->header.precision != 12))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF2.P must be 8 ar 12 (got %hhi)\n",
                     context->header.precision);
          return false;
        }
      if ((context->header.n_component < 1)
          || (context->header.n_component > 4))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG (
              "[D] jpegloader: SOF2.Nf must be in range [1,4] (got %hhi)\n",
              context->header.n_component);
          return false;
        }
      break;

    case 3: // Lossless (sequential)
    case 11:
      if ((context->header.precision < 2) && (context->header.precision > 16))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG (
              "[D] jpegloader: SOF3.P must be in range [2,16] (got %hhi)\n",
              context->header.precision);
          return false;
        }
      if (context->header.n_component < 1)
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG ("[D] jpegloader: SOF3.Nf can not be zero\n");
          return false;
        }
      break;

    default:
      context->error = JPEG_ERROR_NOT_SUPPORTED;
      DEBUG_LOG ("[D] jpegloader: unknown SOF number\n");
      return false;
    }

  // Allocate memory for dynamic part
  context->header.components = (header_component_descriptor_t *)malloc (
      context->header.n_component * sizeof (header_component_descriptor_t));
  if (context->header.components == NULL)
    {
      context->error = JPEG_ERROR_MALLOC;
      DEBUG_LOG ("[D] jpegloader: Can not allocate memory\n");
      return false;
    }

  for (int i = 0; i < context->header.n_component; i++)
    {
      context->header.components[i].component_id = stream[*pos + 8 + i * 3];
      context->header.components[i].sampling = stream[*pos + 9 + i * 3];
      context->header.components[i].dqt_destination
          = stream[*pos + 10 + i * 3];

      // Check for dynamic part
      if ((SAMPLING_H (context->header.components[i].sampling) < 1)
          || (SAMPLING_H (context->header.components[i].sampling) > 4))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG (
              "[D] jpegloader: SOFx.Hi must be in range [1,4] (got %hhi)\n",
              SAMPLING_H (context->header.components[i].sampling));
          return false;
        }
      if ((SAMPLING_V (context->header.components[i].sampling) < 1)
          || (SAMPLING_V (context->header.components[i].sampling) > 4))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          DEBUG_LOG (
              "[D] jpegloader: SOFx.Vi must be in range [1,4] (got %hhi)\n",
              SAMPLING_V (context->header.components[i].sampling));
          return false;
        }

      switch (n)
        {
        case 0: // Baseline DCT
        case 1: // Extended sequential DCT
        case 9:
        case 2: // Progressive DCT
        case 10:
          if (context->header.components[i].dqt_destination > 3)
            {
              context->error = JPEG_ERROR_FILE_NOT_JPEG;
              DEBUG_LOG (
                  "[D] jpegloader: SOF0.Tqi must be in range [0,3] (got "
                  "%hhi)\n",
                  context->header.components[i].dqt_destination);
              return false;
            }
          break;

        case 3: // Lossless (sequential)
        case 11:
          if (context->header.components[i].dqt_destination != 0)
            {
              context->error = JPEG_ERROR_FILE_NOT_JPEG;
              DEBUG_LOG ("[D] jpegloader: SOF1.Tqi must be 1 (got %hhi)\n",
                         context->header.components[i].dqt_destination);
              return false;
            }
          break;
        }
    }

  *pos += block_sz;
  return true;
}
