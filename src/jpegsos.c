#include <jpegloader/jpegtypes.h>

#include "jpegcontext.h"
#include "jpegfunc.h"
#include "jpeglog.h"
#include "jpegsos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* **************************** jpeg_sos_extract *************************** */

bool
jpeg_sos_extract (jpeg_context_t *context, const uint8_t *stream,
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
  size_t scan_sz = 0;

  if (context->scans_count >= 64)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: JPEG can only contains less or equal than 64 "
                "SOS blocks\n");
      return false;
    }

  jpeg_scan_header_t *tmp;
  if (context->scans_count >= context->scans_capacity)
    {
      if (context->scans == NULL)
        {
          context->scans
              = (jpeg_scan_header_t *)malloc (sizeof (jpeg_scan_header_t));
          if (context->scans == NULL)
            {
              context->error = JPEG_ERROR_MALLOC;
              JPEG_LOG ("[E] jpegloader: Can not allocate memory\n");
              return false;
            }
          context->scans_capacity = 1;
        }
      else
        {
          tmp = (jpeg_scan_header_t *)realloc (
              context->scans,
              context->scans_capacity * 2 * sizeof (jpeg_scan_header_t));
          if (tmp == NULL)
            {
              context->error = JPEG_ERROR_MALLOC;
              JPEG_LOG ("[E] jpegloader: Can not allocate memory\n");
              return false;
            }

          context->scans = tmp;
          context->scans_capacity *= 2;
        }
    }

  context->scans[context->scans_count].n_component = *(stream + *pos + 2);
  if ((context->scans[context->scans_count].n_component < 1)
      || (context->scans[context->scans_count].n_component > 4))
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Number of components must be in range [1, 4] "
                "(got %hhi)\n",
                context->scans[context->scans_count].n_component);
      return false;
    }

  for (int i = 0; i < context->scans[context->scans_count].n_component; i++)
    {
      context->scans[context->scans_count].components[i].component_selector
          = *(stream + *pos + 3 + i * 2);
      context->scans[context->scans_count].components[i].scan_destination
          = *(stream + *pos + 4 + i * 2);

      if ((SOS_AC (context->scans[context->scans_count]
                       .components[i]
                       .scan_destination)
           > 3)
          || (SOS_DC (context->scans[context->scans_count]
                          .components[i]
                          .scan_destination)
              > 3))
        {
          context->error = JPEG_ERROR_FILE_NOT_JPEG;
          JPEG_LOG ("[E] jpegloader: Destination can not exceed 3\n");
          return false;
        }
    }
  context->scans[context->scans_count].start
      = *(stream + *pos + 3
          + context->scans[context->scans_count].n_component * 2);
  context->scans[context->scans_count].end
      = *(stream + *pos + 4
          + context->scans[context->scans_count].n_component * 2);
  context->scans[context->scans_count].approximation
      = *(stream + *pos + 5
          + context->scans[context->scans_count].n_component * 2);
  if (SOS_APPROX_H (context->scans[context->scans_count].approximation) > 13)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Approximation hight can not exceed 13\n");
      return false;
    }
  if (SOS_APPROX_L (context->scans[context->scans_count].approximation) > 15)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: Approximation hight can not exceed 13\n");
      return false;
    }

  *pos += block_sz; // Skip block header

  // Forward scan
  uint16_t marker = 0;
  size_t sz = 0;
  size_t i;
  for (i = 0; i < stream_size - *pos; i++)
    {
      if (stream[*pos + i] == 0xFF) // Maybe marker
        {
          if (i == stream_size - *pos - 1)
            {
              context->error = JPEG_ERROR_FILE_NOT_JPEG;
              JPEG_LOG ("[E] jpegloader: End of stream\n");
              return false;
            }
          else
            {
              i++;
              if (stream[*pos + i] == 0) // 0xFF escape
                sz++;
              else // Marker
                break;
            }
        }
      else
        sz++;
    }

  context->scans[context->scans_count].scan_data = NULL;
  DEBUG_LOG ("[D] Scan size is %u\n", (unsigned int)sz);
  context->scans[context->scans_count].scan_length = sz;
  context->scans[context->scans_count].scan_data
      = (uint8_t *)malloc (context->scans[context->scans_count].scan_length);
  if (context->scans[context->scans_count].scan_data == NULL)
    {
      context->error = JPEG_ERROR_MALLOC;
      JPEG_LOG ("[E] jpegloader: Can not allocate memory\n");
      return false;
    }
  sz = 0;
  for (i = 0; i < stream_size - *pos; i++)
    {
      if (stream[*pos + i] == 0xFF) // Maybe marker
        {
          if (i == stream_size - *pos - 1)
            {
              context->error = JPEG_ERROR_FILE_NOT_JPEG;
              JPEG_LOG ("[E] jpegloader: End of stream\n");
              return false;
            }
          else
            {
              i++;
              if (stream[*pos + i] == 0) // 0xFF escape
                {
                  context->scans[context->scans_count].scan_data[sz] = 0xFF;
                  sz++;
                }
              else // Marker
                {
                  break;
                }
            }
        }
      else
        {
          context->scans[context->scans_count].scan_data[sz]
              = stream[*pos + i];
          sz++;
        }
    }

  *pos += i - 1;
  context->scans_count++;

  return true;
}
