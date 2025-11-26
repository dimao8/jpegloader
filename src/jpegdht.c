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
      else
        dht_table[dest].values[i] = NULL; // Keep zero length value arrays NULL
    }

  *pos += block_sz;
  return true;
}

/* ***************************** jpeg_dht_copy ***************************** */

bool
jpeg_dht_copy (jpeg_context_t *context, const jpeg_dht_t *dht, uint8_t class,
               uint8_t destination)
{
  jpeg_dht_t *dest;

  // Check for args
  if ((class > 1) || (destination > 3) || (context == NULL) || (dht == NULL))
    {
      DEBUG_LOG ("[D] class > 1 or destination > 3 or context is NULL or dht "
                 "is NULL\n");
      return false;
    }

  // Check for copy source to source
  if ((class == DHT_CLASS (dht->class_dest))
      && (destination == DHT_DESTINATION (dht->class_dest)))
    {
      DEBUG_LOG ("[D] Source and destination are the same\n");
      return false;
    }

  // Replace
  if (class == DHT_CLASS_DC)
    dest = context->dht_dc + destination;
  else
    dest = context->dht_ac + destination;

  for (int i = 0; i < 16; i++)
    {
      if (dest->values[i] != NULL)
        free (dest->values[i]);
      dest->values[i] = (uint8_t *)malloc (sizeof (uint8_t) * dht->lengths[i]);
      if (dest->values[i] == NULL)
        {
          JPEG_LOG ("[E] Can not allocate memory\n");
          return false;
        }
      memcpy (dest->values[i], dht->values[i],
              sizeof (uint8_t) * dht->lengths[i]);
      dest->lengths[i] = dht->lengths[i];
    }

  dest->class_dest
      = (destination << DHT_DESTINATION_POS) | (class << DHT_CLASS_POS);

  return true;
}
