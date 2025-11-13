#include <jpegloader/jpegtypes.h>

#include "bitstream.h"
#include "huffman.h"
#include "jpegcontext.h"
#include "jpeglog.h"
#include "jpegparser.h"

#include <stdio.h>
#include <stdlib.h>

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

  // TODO : Continue

  free (jpeg_context);
}

/* ***************************** jpeg_decoding ***************************** */

jpeg_error_t
jpeg_decoding (jpeg_context_t *jpeg_context, jpeg_header_t *header,
               void **data)
{
  huffman_node_t *node;
  for (int i = 0; i < 4; i++)
    {
      if (DHT_CLASS (jpeg_context->dht_dc[i].class_dest) != DHT_CLASS_INV)
        {
          DEBUG_LOG ("[D] *************** Huffman table #%i for DC "
                     "coefficients ***************\n",
                     i);
          for (int j = 0; j < 16; j++)
            {
              if (jpeg_context->dht_dc[i].lengths[j] == 0)
                {
                  DEBUG_LOG ("  Empty\n");
                }
              else
                {
                  DEBUG_LOG ("  ");
                  for (int k = 0; k < jpeg_context->dht_dc[i].lengths[j]; k++)
                    {
                      DEBUG_LOG ("0x%02hhx",
                                 jpeg_context->dht_dc[i].values[j][k]);
                      if (k == jpeg_context->dht_dc[i].lengths[j] - 1)
                        {
                          DEBUG_LOG ("\n");
                        }
                      else
                        {
                          DEBUG_LOG (", ");
                        }
                    }
                }
            }

          node = h_create_from_codes (jpeg_context->dht_dc + i);
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in DC #%i\n", i);
              return JPEG_ERROR_HUFFMAN;
            }

          jpeg_context->huffman_tree_dc[i] = node;
        }
    }

  for (int i = 0; i < 4; i++)
    {
      if (DHT_CLASS (jpeg_context->dht_ac[i].class_dest) != DHT_CLASS_INV)
        {
          DEBUG_LOG ("[D] *************** Huffman table #%i for AC "
                     "coefficients ***************\n",
                     i);
          for (int j = 0; j < 16; j++)
            {
              if (jpeg_context->dht_ac[i].lengths[j] == 0)
                {
                  DEBUG_LOG ("  Empty\n");
                }
              else
                {
                  DEBUG_LOG ("  ");
                  for (int k = 0; k < jpeg_context->dht_ac[i].lengths[j]; k++)
                    {
                      DEBUG_LOG ("0x%02hhx",
                                 jpeg_context->dht_ac[i].values[j][k]);
                      if (k == jpeg_context->dht_ac[i].lengths[j] - 1)
                        {
                          DEBUG_LOG ("\n");
                        }
                      else
                        {
                          DEBUG_LOG (", ");
                        }
                    }
                }
            }

          node = h_create_from_codes (jpeg_context->dht_ac + i);
          if (node == NULL)
            {
              JPEG_LOG ("[E] Huffman error in DC #%i\n", i);
              return JPEG_ERROR_HUFFMAN;
            }

          jpeg_context->huffman_tree_ac[i] = node;
        }
    }

  jpeg_bitstream_t *bitstream = bitstream_create (
      jpeg_context->scans[0].scan_data, jpeg_context->scans[0].scan_length);
  if (bitstream == NULL)
    {
      JPEG_LOG ("[E] Huffman error in bitstream\n");
      return JPEG_ERROR_HUFFMAN;
    }

  for (int i = 0; i < 64; i++)
    {
      while (true)
        {
          
        }
    }

  // TODO : Continue

  return JPEG_ERROR_NOT_SUPPORTED;
}
