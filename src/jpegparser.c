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

  for (int i = 0; i < 4; i++)
    {
      h_clear_tree (jpeg_context->huffman_tree_dc[i]);
      h_clear_tree (jpeg_context->huffman_tree_ac[i]);
    }

  // TODO : Continue

  free (jpeg_context);
}

/* *************************** make_from_huffman *************************** */

int
make_from_huffman (uint16_t value, size_t length)
{
  const int mask = (1 << (length - 1));
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
          // DEBUG_LOG ("[D] Huffman DC #%i tree\n", i);
          // h_print_node (stdout, node, 2);

          jpeg_context->huffman_tree_dc[i] = node;
        }

      // h_print_node (stdout, node, 0);
    }

  // h_print_node (stdout, jpeg_context->huffman_tree_dc[1]);

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

  // h_print_node (stdout, jpeg_context->huffman_tree_dc[0]);

  // Create JPEG data bitstream
  jpeg_bitstream_t *bitstream = bitstream_create (
      jpeg_context->scans[0].scan_data, jpeg_context->scans[0].scan_length);
  if (bitstream == NULL)
    {
      JPEG_LOG ("[E] Huffman error in bitstream\n");
      return JPEG_ERROR_HUFFMAN;
    }

  // DeHuffman
  int DCT_Y0[64];
  int DCT_Y1[64];
  int DCT_Cb[64];
  int DCT_Cr[64];

  int Y0[64];
  int Y1[64];
  int Cb[64];
  int Cr[64];

  uint8_t RGB[384];
  uint8_t rle, cat;

  // Y0
  DEBUG_LOG ("[D] Y0\n");
  memset (DCT_Y0, 0, sizeof (int) * 64);
  node = jpeg_context->huffman_tree_dc[SOS_AC(jpeg_context->scans[0].components[0].scan_destination)];
  while (true)
    {
      node = h_move (node, bitstream_next_bit (bitstream));
      if (node == NULL)
        {
          JPEG_LOG ("[E] Huffman error in bitstream\n");
          return JPEG_ERROR_HUFFMAN;
        }
      else if (h_is_leaf (node))
        {
          val = bitstream_extract (bitstream, node->value);
          DCT_Y0[dezigzaging (0)] = make_from_huffman (val, node->value)
                                    * jpeg_context->dqt[0].table[0];
          break;
        }
    }

  for (int i = 1; i < 64; i++)
    {
      node = jpeg_context->huffman_tree_ac[0];
      while (true)
        {
          node = h_move (node, bitstream_next_bit (bitstream));
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in bitstream\n");
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
                      i += 16;
                      break;
                    }
                  else
                    {
                      i = 64;
                      break;
                    }
                }
              else
                {
                  i += rle;
                  if (i >= 64)
                    break;
                  val = bitstream_extract (bitstream, cat);
                  DCT_Y0[dezigzaging (i)] = make_from_huffman (val, cat)
                                            * jpeg_context->dqt[0].table[i];
                }
              break;
            }
        }
    }

  idct (Y0, DCT_Y0);

  for (int j = 0; j < 8; j++)
    {
      DEBUG_LOG ("[D] ");
      for (int i = 0; i < 8; i++)
        {
          DEBUG_LOG ("%04i", DCT_Y0[j * 8 + i]);
          if (i == 7)
            {
              DEBUG_LOG ("\n");
            }
          else
            {
              DEBUG_LOG (" ");
            }
        }
    }

  // Y1
  memset (DCT_Y1, 0, sizeof (int) * 64);
  DEBUG_LOG ("[D] Y1\n");
  node = jpeg_context->huffman_tree_dc[0];
  while (true)
    {
      node = h_move (node, bitstream_next_bit (bitstream));
      if (node == NULL)
        {
          JPEG_LOG ("[E] Huffman error in bitstream\n");
          return JPEG_ERROR_HUFFMAN;
        }
      else if (h_is_leaf (node))
        {
          val = bitstream_extract (bitstream, node->value);
          DCT_Y1[dezigzaging (0)] = make_from_huffman (val, node->value)
                                        * jpeg_context->dqt[0].table[0]
                                    + DCT_Y0[dezigzaging (0)];
          break;
        }
    }

  for (int i = 1; i < 64; i++)
    {
      node = jpeg_context->huffman_tree_ac[0];
      while (true)
        {
          node = h_move (node, bitstream_next_bit (bitstream));
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in bitstream\n");
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
                      i += 16;
                      break;
                    }
                  else
                    {
                      i = 64;
                      break;
                    }
                }
              else
                {
                  i += rle;
                  if (i >= 64)
                    break;
                  val = bitstream_extract (bitstream, cat);
                  DCT_Y1[dezigzaging (i)] = make_from_huffman (val, cat)
                                            * jpeg_context->dqt[0].table[i];
                  break;
                }
            }
        }
    }

  idct (Y1, DCT_Y1);

  for (int j = 0; j < 8; j++)
    {
      DEBUG_LOG ("[D] ");
      for (int i = 0; i < 8; i++)
        {
          DEBUG_LOG ("%04i", DCT_Y1[j * 8 + i]);
          if (i == 7)
            {
              DEBUG_LOG ("\n");
            }
          else
            {
              DEBUG_LOG (" ");
            }
        }
    }

  // Cb
  memset (DCT_Cb, 0, sizeof (int) * 64);
  DEBUG_LOG ("[D] DCT Cb0\n");
  node = jpeg_context->huffman_tree_dc[1];
  while (true)
    {
      node = h_move (node, bitstream_next_bit (bitstream));
      if (node == NULL)
        {
          JPEG_LOG ("[E] Huffman error in bitstream\n");
          return JPEG_ERROR_HUFFMAN;
        }
      else if (h_is_leaf (node))
        {
          val = bitstream_extract (bitstream, node->value);
          DCT_Cb[dezigzaging (0)] = make_from_huffman (val, node->value)
                                    * jpeg_context->dqt[1].table[0];
          break;
        }
    }

  for (int i = 1; i < 64; i++)
    {
      node = jpeg_context->huffman_tree_ac[1];
      while (true)
        {
          node = h_move (node, bitstream_next_bit (bitstream));
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in bitstream\n");
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
                      i += 16;
                      break;
                    }
                  else
                    {
                      i = 64;
                      break;
                    }
                }
              else
                {
                  i += rle;
                  if (i >= 64)
                    break;
                  val = bitstream_extract (bitstream, cat);
                  DCT_Cb[dezigzaging (i)] = make_from_huffman (val, cat)
                                            * jpeg_context->dqt[1].table[i];
                  break;
                }
            }
        }
    }

  idct (Cb, DCT_Cb);

  for (int j = 0; j < 8; j++)
    {
      DEBUG_LOG ("[D] ");
      for (int i = 0; i < 8; i++)
        {
          DEBUG_LOG ("%04i", DCT_Cb[j * 8 + i]);
          if (i == 7)
            {
              DEBUG_LOG ("\n");
            }
          else
            {
              DEBUG_LOG (" ");
            }
        }
    }

  // Cr
  memset (DCT_Cr, 0, sizeof (int) * 64);
  DEBUG_LOG ("[D] DCT Cr0\n");
  node = jpeg_context->huffman_tree_dc[1];
  while (true)
    {
      node = h_move (node, bitstream_next_bit (bitstream));
      if (node == NULL)
        {
          JPEG_LOG ("[E] Huffman error in bitstream\n");
          return JPEG_ERROR_HUFFMAN;
        }
      else if (h_is_leaf (node))
        {
          val = bitstream_extract (bitstream, node->value);
          DCT_Cr[dezigzaging (0)] = make_from_huffman (val, node->value)
                                    * jpeg_context->dqt[1].table[0];
          break;
        }
    }

  for (int i = 1; i < 64; i++)
    {
      node = jpeg_context->huffman_tree_ac[1];
      while (true)
        {
          node = h_move (node, bitstream_next_bit (bitstream));
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in bitstream\n");
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
                      i += 16;
                      break;
                    }
                  else
                    {
                      i = 64;
                      break;
                    }
                }
              else
                {
                  i += rle;
                  if (i >= 64)
                    break;
                  val = bitstream_extract (bitstream, cat);
                  DCT_Cr[dezigzaging (i)] = make_from_huffman (val, cat)
                                            * jpeg_context->dqt[1].table[i];
                  break;
                }
            }
        }
    }

  idct (Cr, DCT_Cr);

  for (int j = 0; j < 8; j++)
    {
      DEBUG_LOG ("[D] ");
      for (int i = 0; i < 8; i++)
        {
          DEBUG_LOG ("%04i", DCT_Cr[j * 8 + i]);
          if (i == 7)
            {
              DEBUG_LOG ("\n");
            }
          else
            {
              DEBUG_LOG (" ");
            }
        }
    }

  for (int j = 0; j < 8; j++)
    {
      for (int i = 0; i < 8; i++)
        {
          RGB[(j * 16 + i) * 3] = clampi (
              Y0[j * 8 + i] + (1.402f * (Cr[j * 8 + i / 2] - 128)), 0, 255);
          RGB[(j * 16 + i + 8) * 3] = clampi (
              Y1[j * 8 + i] + (1.402f * (Cr[j * 8 + i / 2 + 4] - 128)), 0,
              255);

          RGB[(j * 16 + i) * 3 + 1]
              = clampi (Y0[j * 8 + i] - (0.34414 * (Cb[j * 8 + i / 2] - 128))
                            - (0.71414 * (Cr[j * 8 + i / 2] - 128)),
                        0, 255);
          RGB[(j * 16 + i + 8) * 3 + 1] = clampi (
              Y1[j * 8 + i] - (0.34414 * (Cb[j * 8 + i / 2 + 4] - 128))
                  - (0.71414 * (Cr[j * 8 + i / 2 + 4] - 128)),
              0, 255);

          RGB[(j * 16 + i) * 3 + 2] = clampi (
              Y0[j * 8 + i] + (1.772 * (Cb[j * 8 + i / 2] - 128)), 0, 255);
          RGB[(j * 16 + i + 8) * 3 + 2] = clampi (
              Y1[j * 8 + i] + (1.772 * (Cb[j * 8 + i / 2 + 4] - 128)), 0, 255);
        }
    }

  make_tga (RGB, 16, 8, "result.tga");

  // TODO : Continue

  return JPEG_ERROR_NOT_SUPPORTED;
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
