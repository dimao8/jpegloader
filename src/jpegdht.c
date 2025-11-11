#include <jpegloader/jpegtypes.h>

#include "jpegcontext.h"
#include "jpegdht.h"
#include "jpegfunc.h"
#include "jpeglog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* **************************** jpeg_dht_extract *************************** */

bool
jpeg_dht_extract (jpeg_context_t *context, const uint8_t *stream,
                  size_t stream_size, size_t *pos)
{
  if (context == NULL || stream == NULL || pos == NULL
      || (*pos >= stream_size))
    return false;

  if (*pos >= stream_size - 19)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: End of stream\n");
      return false;
    }

  size_t block_sz = ntohs (*(uint16_t *)(stream + *pos));
  uint8_t class = DHT_CLASS (*(stream + *pos + 2));
  uint8_t dest = DHT_DESTINATION (*(stream + *pos + 2));
  jpeg_dht_t *dht_table;
  size_t val_sz = 0;

  if (class == 0)
    dht_table = context->dht_dc;
  else if (class == 1)
    dht_table = context->dht_ac;
  else
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: DHT class must be only 0 or 1 (got %hhi)\n",
                class);
      return false;
    }
  if (dest > 3)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: DHT destination must be in range [0, 3] (got "
                "%hhi)\n",
                dest);
      return false;
    }

  if (DHT_DESTINATION (dht_table[dest].class_dest)
      != DHT_DESTINATION_INV) // Allready init
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: DHT table with class=%hhi, destination=%hhi "
                "allready exist\n",
                class, dest);
      return false;
    }
  dht_table[dest].class_dest = *(stream + *pos + 2);

  for (int i = 0; i < 16; i++)
    {
      dht_table[dest].lengths[i] = (stream + *pos + 3)[i];
      val_sz += dht_table[dest].lengths[i];
    }
  if (*pos >= stream_size - 19 - val_sz)
    {
      context->error = JPEG_ERROR_FILE_NOT_JPEG;
      JPEG_LOG ("[E] jpegloader: End of stream\n");
      return false;
    }

  val_sz = 0;
  for (int i = 0; i < 16; i++)
    {
      if (dht_table[dest].lengths[i] != 0)
        {
          dht_table[dest].values[i]
              = (uint8_t *)malloc (dht_table[dest].lengths[i]);
          if (dht_table[dest].values[i] == NULL)
            {
              context->error = JPEG_ERROR_MALLOC;
              JPEG_LOG ("[E] jpegloader: Can not allocate memory\n");
              return false;
            }
          memcpy (dht_table[dest].values[i], stream + *pos + 19 + val_sz,
                  dht_table[dest].lengths[i]);
          val_sz += dht_table[dest].lengths[i];
        }
    }

  *pos += block_sz;
  return true;
}
