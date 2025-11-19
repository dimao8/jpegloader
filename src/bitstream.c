#include "bitstream.h"
#include "jpegfunc.h"
#include "jpeglog.h"

#include <stdbool.h>
#include <stdlib.h>

/**
 * \private
 */
typedef struct jpeg_bitstream_tag
{
  uint8_t *bytes;
  size_t byte_length;
  size_t byte_pos;
  size_t bit_offset;
} jpeg_bitstream_t;

/* **************************** bitstream_create *************************** */

jpeg_bitstream_t *
bitstream_create (uint8_t *pointer, size_t size)
{
  if ((size == 0) || (pointer == NULL))
    return NULL;

  jpeg_bitstream_t *stream
      = (jpeg_bitstream_t *)malloc (sizeof (jpeg_bitstream_t));
  if (stream == NULL)
    return NULL;

  stream->bytes = pointer;
  stream->byte_length = size;
  bitstream_reset (stream);

  return stream;
}

/* **************************** bitstream_reset **************************** */

void
bitstream_reset (jpeg_bitstream_t *stream)
{
  if (stream == NULL)
    return;

  stream->byte_pos = 0;
  stream->bit_offset = 0;
}

/* *************************** bitstream_extract *************************** */

uint32_t
bitstream_extract (jpeg_bitstream_t *stream, size_t length)
{
  uint16_t tmp = 0;
  size_t n = 0;
  size_t i = 0;

  if ((stream == NULL)
      || (length > 16)) // Stream is NULL or length is incorrect
    return BITSTREAM_EOS;

  if (length
      > ((stream->byte_length - stream->byte_pos) * 8) - stream->bit_offset)
    return BITSTREAM_EOS;

  if (length == 0)
    return 0;

  tmp = (uint16_t)(stream->bytes[stream->byte_pos + i])
        << (stream->bit_offset + 8);
  i++;
  if ((stream->byte_pos + i) < stream->byte_length)
    {
      tmp |= (uint16_t)(stream->bytes[stream->byte_pos + i])
             << stream->bit_offset;
    }
  i++;
  if ((stream->byte_pos + i) < stream->byte_length)
    {
      tmp |= (uint16_t)(stream->bytes[stream->byte_pos + i])
             >> (8 - stream->bit_offset);
    }
  i++;

  tmp >>= 16 - length;
  stream->byte_pos += (stream->bit_offset + length) / 8;
  stream->bit_offset = (stream->bit_offset + length) % 8;

  return tmp;
}

/* *************************** bitstream_next_bit ************************** */

uint32_t
bitstream_next_bit (jpeg_bitstream_t *stream)
{
  if (stream == NULL)
    return BITSTREAM_EOS;

  if (stream->byte_pos >= stream->byte_length)
    return BITSTREAM_EOS;

  uint32_t tmp
      = (stream->bytes[stream->byte_pos] >> (7 - stream->bit_offset)) & 1;

  stream->bit_offset++;
  if (stream->bit_offset >= 8)
    {
      stream->bit_offset = 0;
      stream->byte_pos++;
    }

  return tmp;
}

/* ********************** bitstream_print_near_current ********************* */

void
bitstream_print_near_current (jpeg_bitstream_t *stream)
{
  if (stream == NULL)
    {
      DEBUG_LOG ("[D] stream is NULL\n");
      return;
    }

  if (stream->byte_length == 0)
    {
      DEBUG_LOG ("[D] stream is empty\n");
      return;
    }

  DEBUG_LOG ("[D] stream near [%zi]: ", stream->byte_pos);
  int start = clampi (stream->byte_pos - 2, 0, stream->byte_pos);
  int end
      = clampi (stream->byte_pos + 2, stream->byte_pos, stream->byte_length);
  for (int i = start; i < end; i++)
    {
      DEBUG_LOG ("0x%02hhx", stream->bytes[i]);
      if (i != end - 1)
        {
          DEBUG_LOG (" ");
        }
      else
        {
          DEBUG_LOG ("\n");
        }
    }
}
