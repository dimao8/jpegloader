#include <jpegloader/jpegtypes.h>

#include "bitstream.h"
#include "huffman.h"
#include "idct.h"
#include "jpegcontext.h"
#include "jpegfunc.h"
#include "jpeglog.h"
#include "jpegparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int make_from_huffman (uint16_t value, size_t length);
bool make_jpeg_component_descriptor (jpeg_component_descriptor_t *descriptor,
                                     size_t scan_component_index,
                                     jpeg_context_t *ctx);
jpeg_error_t jpeg_process_buffer (jpeg_block_buffer_t *buffer,
                                  jpeg_context_t *ctx,
                                  jpeg_bitstream_t *bitstream);
jpeg_error_t jpeg_install_mcu (jpeg_context_t *ctx, size_t bx, size_t by,
                               size_t x, size_t y);
void make_tga (const uint8_t *data, size_t width, size_t height,
               const char *path);

/* ******************************* jpeg_init ******************************* */

jpeg_context_t *
jpeg_init ()
{
  jpeg_context_t *ctx = (jpeg_context_t *)malloc (sizeof (jpeg_context_t));

  if (ctx == NULL)
    return NULL;

  ctx->error = JPEG_NO_ERROR;

  // Init Header
  ctx->header.precision = 0;
  ctx->header.width = 0;
  ctx->header.height = 0;
  ctx->header.n_component = 0;
  ctx->header.components = NULL;

  // Init app section
  for (int i = 0; i < 16; i++)
    {
      ctx->app[i].data = NULL;
      ctx->app[i].size = 0;
    }

  // Init DQT
  for (int i = 0; i < 4; i++)
    {
      ctx->dqt[i].prec_dest = 0xFF;
      for (int j = 0; j < 64; j++)
        ctx->dqt[i].table[j] = 0;
    }

  // Init DHT
  for (int i = 0; i < 4; i++)
    {
      // DC values
      ctx->dht_dc[i].class_dest
          = (DHT_CLASS_INV << DHT_CLASS_POS)
            | (DHT_DESTINATION_INV << DHT_DESTINATION_POS);
      for (int j = 0; j < 16; j++)
        {
          ctx->dht_dc[i].lengths[j] = 0;
          ctx->dht_dc[i].values[j] = NULL;
        }

      // AC values
      ctx->dht_ac[i].class_dest
          = (DHT_CLASS_INV << DHT_CLASS_POS)
            | (DHT_DESTINATION_INV << DHT_DESTINATION_POS);
      for (int j = 0; j < 16; j++)
        {
          ctx->dht_ac[i].lengths[j] = 0;
          ctx->dht_ac[i].values[j] = NULL;
        }
    }

  // Scans
  ctx->scans = NULL;
  ctx->scans_capacity = 0;
  ctx->scans_count = 0;

  // Comment
  ctx->comment = NULL;

  // Buffers
  ctx->n_buffers = 0;
  ctx->mcu_buffers = NULL;

  // RGB
  ctx->rgb = NULL;

  // TODO : Continue

  return ctx;
}

/* ******************************* jpeg_free ******************************* */

void
jpeg_free_context (jpeg_context_t *jpeg_context)
{
  if (jpeg_context == 0)
    return;

  if (jpeg_context->header.components != NULL)
    free (jpeg_context->header.components);

  for (int i = 0; i < 16; i++)
    {
      if (jpeg_context->app[i].data != NULL)
        free (jpeg_context->app[i].data);
      jpeg_context->app[i].size = 0;
    }

  for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 16; j++)
        {
          jpeg_context->dht_dc[i].lengths[j] = 0;
          if (jpeg_context->dht_dc[i].values[j] != NULL)
            free (jpeg_context->dht_dc[i].values[j]);

          jpeg_context->dht_ac[i].lengths[j] = 0;
          if (jpeg_context->dht_ac[i].values[j] != NULL)
            free (jpeg_context->dht_ac[i].values[j]);
        }
    }

  if (jpeg_context->scans != NULL)
    {
      for (int i = 0; i < jpeg_context->scans_count; i++)
        {
          if (jpeg_context->scans[i].scan_data != NULL)
            free (jpeg_context->scans[i].scan_data);
        }
      free (jpeg_context->scans);
      jpeg_context->scans_capacity = 0;
      jpeg_context->scans_count = 0;
    }

  if (jpeg_context->comment != NULL)
    free (jpeg_context->comment);

  for (int i = 0; i < 4; i++)
    {
      h_clear_tree (jpeg_context->huffman_tree_dc[i]);
      h_clear_tree (jpeg_context->huffman_tree_ac[i]);
    }

  if (jpeg_context->mcu_buffers != NULL)
    free (jpeg_context->mcu_buffers);

  if (jpeg_context->rgb != NULL)
    free (jpeg_context->rgb);

  // TODO : Continue

  free (jpeg_context);
}

/* *************************** make_from_huffman *************************** */

int
make_from_huffman (uint16_t value, size_t length)
{
  const uint16_t mask = (1 << (length - 1));

  if (length == 0)
    return 0;

  if (value & mask)
    return value;
  else
    return (int)(value) - (1L << length) + 1;
}

/* ***************************** jpeg_decoding ***************************** */

jpeg_error_t
jpeg_decoding (jpeg_context_t *jpeg_context, jpeg_header_t *header,
               void **data)
{
  huffman_node_t *node;
  int val;

  // Create DC Huffman trees
  for (int i = 0; i < 4; i++)
    {
      if (DHT_CLASS (jpeg_context->dht_dc[i].class_dest) != DHT_CLASS_INV)
        {
          node = h_create_from_codes (jpeg_context->dht_dc + i);
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in DC #%i\n", i);
              return JPEG_ERROR_HUFFMAN;
            }

          jpeg_context->huffman_tree_dc[i] = node;
        }
    }

  // Create AC Huffman trees
  for (int i = 0; i < 4; i++)
    {
      if (DHT_CLASS (jpeg_context->dht_ac[i].class_dest) != DHT_CLASS_INV)
        {
          node = h_create_from_codes (jpeg_context->dht_ac + i);
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in AC #%i\n", i);
              return JPEG_ERROR_HUFFMAN;
            }

          jpeg_context->huffman_tree_ac[i] = node;
        }
    }

  // Create JPEG data bitstream
  jpeg_bitstream_t *bitstream = bitstream_create (
      jpeg_context->scans[0].scan_data, jpeg_context->scans[0].scan_length);
  if (bitstream == NULL)
    {
      JPEG_LOG ("[E] Huffman error in bitstream\n");
      return JPEG_ERROR_HUFFMAN;
    }

  // Create image
  jpeg_context->rgb = (uint8_t *)malloc (jpeg_context->header.width
                                         * jpeg_context->header.height * 3);
  if (jpeg_context->rgb == NULL)
    {
      JPEG_LOG ("[E] Can not allocate memory\n");
      return JPEG_ERROR_MALLOC;
    }

  // Get number of blocks in MCU
  size_t n = 0;
  size_t hs = 0;
  size_t vs = 0;

  for (int i = 0; i < jpeg_context->header.n_component; i++)
    {
      n += SAMPLING_H (jpeg_context->header.components[i].sampling)
           * SAMPLING_V (jpeg_context->header.components[i].sampling);
      if (SAMPLING_H (jpeg_context->header.components[i].sampling) > hs)
        hs = SAMPLING_H (jpeg_context->header.components[i].sampling);
      if (SAMPLING_V (jpeg_context->header.components[i].sampling) > vs)
        vs = SAMPLING_V (jpeg_context->header.components[i].sampling);
    }
  jpeg_context->n_buffers = n;
  jpeg_context->mcu_buffers
      = (jpeg_block_buffer_t *)malloc (sizeof (jpeg_block_buffer_t) * n);
  if (jpeg_context->mcu_buffers == NULL)
    {
      JPEG_LOG ("[E] Can not allocate memory\n");
      return JPEG_ERROR_MALLOC;
    }

  // Filling blocks
  n = 0;
  for (int i = 0; i < jpeg_context->header.n_component; i++)
    {
      for (int k = 0;
           k < SAMPLING_V (jpeg_context->header.components[i].sampling); k++)
        {
          for (int j = 0;
               j < SAMPLING_H (jpeg_context->header.components[i].sampling);
               j++)
            {
              make_jpeg_component_descriptor (
                  &(jpeg_context->mcu_buffers[n].descriptor), i, jpeg_context);
              n++;
            }
        }
    }

  size_t nx = (jpeg_context->header.width + hs * 8 - 1) / (hs * 8);
  size_t ny = (jpeg_context->header.height + vs * 8 - 1) / (vs * 8);
  jpeg_error_t err;

  for (int j = 0; j < ny; j++)
    {
      for (int i = 0; i < nx; i++)
        {
          for (int k = 0; k < jpeg_context->n_buffers; k++)
            {
              err = jpeg_process_buffer (jpeg_context->mcu_buffers + k,
                                         jpeg_context, bitstream);

              if (err != JPEG_NO_ERROR)
                {
                  JPEG_LOG ("[E] Error while decoding MCU[%i, %i]\n", i, j);
                  return err;
                }
            }
          jpeg_install_mcu (jpeg_context, hs, vs, i * hs * 8, j * vs * 8);
        }
    }

  make_tga (jpeg_context->rgb, jpeg_context->header.width,
            jpeg_context->header.height, "result.tga");

  return JPEG_NO_ERROR;
}

/* ********************* make_jpeg_component_descriptor ******************** */

bool
make_jpeg_component_descriptor (jpeg_component_descriptor_t *descriptor,
                                size_t scan_component_index,
                                jpeg_context_t *ctx)
{
  // TODO : multiple scans

  header_component_descriptor_t *hdr_descriptor = NULL;

  descriptor->component_index
      = ctx->scans[0].components[scan_component_index].component_selector;
  descriptor->h_dc_table_index = SOS_DC (
      ctx->scans[0].components[scan_component_index].scan_destination);
  descriptor->h_ac_table_index = SOS_AC (
      ctx->scans[0].components[scan_component_index].scan_destination);

  for (int i = 0; i < ctx->header.n_component; i++)
    {
      if (ctx->header.components[i].component_id
          == descriptor->component_index)
        {
          hdr_descriptor = ctx->header.components + i;
          break;
        }
    }

  if (hdr_descriptor == NULL)
    return false;

  descriptor->q_table_index = hdr_descriptor->dqt_destination;
  descriptor->h_number = SAMPLING_H (hdr_descriptor->sampling);
  descriptor->v_number = SAMPLING_V (hdr_descriptor->sampling);

  return true;
}

/* ************************** jpeg_process_buffer ************************** */

jpeg_error_t
jpeg_process_buffer (jpeg_block_buffer_t *buffer, jpeg_context_t *ctx,
                     jpeg_bitstream_t *bitstream)
{
  huffman_node_t *node;
  uint32_t val;
  uint8_t cat;
  uint8_t rle;
  size_t index;
  int itmp;

  for (index = 0; index < ctx->header.n_component; index++)
    {
      if (ctx->header.components[index].component_id
          == buffer->descriptor.component_index)
        break;
    }

  memset (buffer->dct_data, 0, sizeof (int) * 64);
  node = ctx->huffman_tree_dc[buffer->descriptor.h_dc_table_index];
  while (true)
    {
      node = h_move (node, bitstream_next_bit (bitstream));
      if (node == NULL)
        {
          JPEG_LOG ("[E] h_move: Huffman error in bitstream\n");
          return JPEG_ERROR_HUFFMAN;
        }
      else if (h_is_leaf (node))
        {
          val = bitstream_extract (bitstream, node->value);
          if (val == BITSTREAM_EOS)
            {
              JPEG_LOG ("[E] bitstream_extract: Huffman error in bitstream\n");
              return JPEG_ERROR_HUFFMAN;
            }
          // FIXME : Not an index - 1 but search for DC cache in header
          itmp = make_from_huffman (val, node->value);
          itmp
              += ctx->header.components[buffer->descriptor.component_index - 1]
                     .dc_cache;
          buffer->dct_data[dezigzaging (0)]
              = itmp * ctx->dqt[buffer->descriptor.q_table_index].table[0];
          ctx->header.components[buffer->descriptor.component_index - 1]
              .dc_cache
              = itmp;
          break;
        }
    }

  for (int i = 1; i < 64; i++)
    {
      node = ctx->huffman_tree_ac[buffer->descriptor.h_ac_table_index];
      while (true)
        {
          node = h_move (node, bitstream_next_bit (bitstream));
          if (node == NULL)
            {
              JPEG_LOG ("[E] h_move: Huffman error in bitstream\n");
              return JPEG_ERROR_HUFFMAN;
            }
          else if (h_is_leaf (node))
            {
              cat = node->value;
              rle = H_RLE (cat);
              cat = H_CATEGORY (cat);
              if (cat == 0)
                {
                  if (rle == 15)
                    {
                      i += 15;
                      break;
                    }
                  else if (rle == 0)
                    {
                      i = 64;
                      break;
                    }
                  else
                    {
                      JPEG_LOG (
                          "[E] Wrong RLE/Category pair in Huffman table\n");
                      return JPEG_ERROR_FILE_NOT_JPEG;
                    }
                }
              else
                {
                  i += rle;
                  if (i >= 64)
                    break;
                  val = bitstream_extract (bitstream, cat);
                  buffer->dct_data[dezigzaging (i)]
                      = make_from_huffman (val, cat)
                        * ctx->dqt[buffer->descriptor.q_table_index].table[i];
                  break;
                }
            }
        }
    }

  idct (buffer->data, buffer->dct_data);

  return JPEG_NO_ERROR;
}

/* **************************** jpeg_install_mcu *************************** */

jpeg_error_t
jpeg_install_mcu (jpeg_context_t *ctx, size_t bx, size_t by, size_t x,
                  size_t y)
{
  const size_t Yx = SAMPLING_H (ctx->header.components[0].sampling);
  const size_t Yy = SAMPLING_V (ctx->header.components[0].sampling);
  const size_t Ys = Yx * Yy;
  const size_t Cbx = SAMPLING_H (ctx->header.components[1].sampling);
  const size_t Cby = SAMPLING_V (ctx->header.components[1].sampling);
  const size_t Cbs = Cbx * Cby;
  const size_t Crx = SAMPLING_H (ctx->header.components[2].sampling);
  const size_t Cry = SAMPLING_V (ctx->header.components[2].sampling);
  const size_t Crs = Crx * Cry;

  size_t Yn;
  size_t Cbn;
  size_t Crn;

  int Y;
  int Cb;
  int Cr;

  int yi, yj;
  int cbi, cbj;
  int cri, crj;

  for (int j = 0; j < 8 * by; j++)
    {
      for (int i = 0; i < 8 * bx; i++)
        {
          if ((x + i < ctx->header.width) && ((y + j < ctx->header.height)))
            {
              Yn = ((j / 8) * Yx + i / 8) % Ys;
              Cbn = ((j / 8) * Cbx + i / 8) % Cbs;
              Crn = ((j / 8) * Crx + i / 8) % Crs;

              yi = (Yx * i / bx) % 8;
              yj = (Yy * j / by) % 8;
              cbi = (Cbx * i / bx) % 8;
              cbj = (Cby * j / by) % 8;
              cri = (Crx * i / bx) % 8;
              crj = (Cry * j / by) % 8;

              Y = ctx->mcu_buffers[Yn].data[yj * 8 + yi];
              Cb = ctx->mcu_buffers[Ys + Cbn].data[cbj * 8 + cbi];
              Cr = ctx->mcu_buffers[Ys + Cbs + Crn].data[crj * 8 + cri];

              ctx->rgb[((y + j) * ctx->header.width + i + x) * 3]
                  = clampi (Y + (1.402f * (Cr - 128)), 0, 255);
              ctx->rgb[((y + j) * ctx->header.width + i + x) * 3 + 1]
                  = clampi (Y - (0.34414f * (Cb - 128))
                                - (0.71414f * (Cr - 128)),
                            0, 255);
              ctx->rgb[((y + j) * ctx->header.width + i + x) * 3 + 2]
                  = clampi (Y + (1.772f * (Cb - 128)), 0, 255);
            }
        }
    }

  return JPEG_NO_ERROR;
}

/* ******************************** make_tga ******************************* */

void
make_tga (const uint8_t *data, size_t width, size_t height, const char *path)
{
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;

#pragma pack(push, 1)
  typedef struct tga_header_tag
  {
    uint8_t length;
    uint8_t colormap_type;
    uint8_t image_type;
    uint16_t first_entry_index;
    uint16_t colormap_length;
    uint8_t colormap_entry_size;
    uint16_t xorigin;
    uint16_t yorigin;
    uint16_t width;
    uint16_t height;
    uint8_t depth;
    uint8_t descriptor;
  } tga_header_t;
#pragma pack(pop)

  tga_header_t header;

  FILE *f = fopen (path, "wb");
  if (f == NULL)
    return;

  header.length = 0;
  header.colormap_type = 0;
  header.image_type = 2;
  header.first_entry_index = 0;
  header.colormap_length = 0;
  header.colormap_entry_size = 0;
  header.xorigin = 0;
  header.yorigin = 0;
  header.width = width;
  header.height = height;
  header.depth = 24;
  header.descriptor = 0;
  fwrite (&header, sizeof (tga_header_t), 1, f);
  for (size_t j = 0; j < height; j++)
    {
      for (size_t i = 0; i < width; i++)
        {
          fwrite (data + ((height - j - 1) * width + i) * 3 + 2, 1, 1, f);
          fwrite (data + ((height - j - 1) * width + i) * 3 + 1, 1, 1, f);
          fwrite (data + ((height - j - 1) * width + i) * 3, 1, 1, f);
        }
    }

  fclose (f);
}
