#include "bitstream.h"
#include "jpeglog.h"

#include <stdio.h>
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
  uint16_t tmp;

  if ((stream == NULL) || (length == 0) || (length > 16))
    return BITSTREAM_EOS;

  const uint8_t mask = 0xFF >> stream->bit_offset;

  switch (stream->byte_length - stream->byte_pos)
    {
    case 1: // Extract one byte and fill others with 0
      DEBUG_LOG ("[D] Extract one byte and fill others with 0\n");
      tmp = ((uint16_t)stream->bytes[stream->byte_pos] & mask)
            << (stream->bit_offset + 8);
      break;

    case 2: // Extract two bytes and fill others with 0
      DEBUG_LOG ("[D] Extract two bytes and fill others with 0\n");
      tmp = ((uint16_t)stream->bytes[stream->byte_pos] & mask)
            << (stream->bit_offset + 8);
      tmp |= ((uint16_t)stream->bytes[stream->byte_pos + 1]
              << stream->bit_offset);
      break;

    default: // Extract three bytes
      DEBUG_LOG ("[D] Extract three bytes\n");
      tmp = ((uint16_t)stream->bytes[stream->byte_pos] & mask)
            << (stream->bit_offset + 8);
      tmp |= ((uint16_t)stream->bytes[stream->byte_pos + 1]
              << stream->bit_offset);
      tmp |= ((uint16_t)stream->bytes[stream->byte_pos + 2]
              >> (8 - stream->bit_offset));
      break;
    }

  tmp >>= 16 - length;
  size_t step = stream->bit_offset + length;
  stream->bit_offset = (stream->bit_offset + step) % 8;
  stream->byte_pos += step / 8;

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
