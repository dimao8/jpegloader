#include "bitstream.h"
#include "jpegfunc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

/* ******************************* print_bits ****************************** */

void
print_bits (FILE *where, uint32_t value, size_t sz)
{
  sz = clampi (sz, 0, 32);

  uint32_t mask = (1 << (sz - 1));

  for (int i = 0; i < sz; i++)
    {
      fputc (((value & mask) == 0) ? '0' : '1', where);
      value <<= 1;
    }
}

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
bitstream_print_state (FILE *where, jpeg_bitstream_t *stream)
{
  if (stream == NULL)
    {
      fprintf (where, "stream is NULL\n");
      return;
    }

  fprintf (where, "stream->byte_length: %zi\n", stream->byte_length);
  fprintf (where, "stream->byte_pos: %zi\n", stream->byte_pos);
  fprintf (where, "stream->bit_offset: %zi\n", stream->bit_offset);

  if (stream->byte_pos == stream->byte_length)
    {
      fprintf (where, "OUT OF STREAM\n");
      return;
    }

  if (stream->byte_pos == 0)
    {
      print_bits (where, stream->bytes[0], 8);
      fputc (' ', where);
      print_bits (where, stream->bytes[1], 8);
      fputc ('\n', where);
      for (int i = 0; i < stream->bit_offset; i++)
        fputc ('-', where);
      fprintf (where, "^\n");
    }
  else if (stream->byte_pos == stream->byte_length - 1)
    {
      print_bits (where, stream->bytes[stream->byte_length - 2], 8);
      fputc (' ', where);
      print_bits (where, stream->bytes[stream->byte_length - 1], 8);
      fputc ('\n', where);
      for (int i = 0; i < stream->bit_offset + 9; i++)
        fputc ('-', where);
      fprintf (where, "^\n");
    }
  else
    {
      print_bits (where, stream->bytes[stream->byte_pos - 1], 8);
      fputc (' ', where);
      print_bits (where, stream->bytes[stream->byte_pos], 8);
      fputc (' ', where);
      print_bits (where, stream->bytes[stream->byte_pos + 1], 8);
      fputc ('\n', where);
      for (int i = 0; i < stream->bit_offset + 9; i++)
        fputc ('-', where);
      fprintf (where, "^\n");
    }
}
