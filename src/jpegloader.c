#include <jpegloader/jpegloader.h>

#include "jpegapp.h"
#include "jpegcom.h"
#include "jpegcontext.h"
#include "jpegdht.h"
#include "jpegdqt.h"
#include "jpegloader/jpegtypes.h"
#include "jpeglog.h"
#include "jpegparser.h"
#include "jpegsof.h"
#include "jpegsos.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum jpeg_segment_id_tag
{
  TEM = 1,
  SOF0 = 192,
  SOF1 = 193,
  SOF2 = 194,
  SOF3 = 195,
  DHT = 196,
  SOF4 = 197,
  SOF5 = 198,
  SOF6 = 199,
  JPG0 = 200,
  SOF7 = 201,
  SOF8 = 202,
  SOF9 = 203,
  DAC = 204,
  SOF10 = 205,
  SOF11 = 206,
  SOF12 = 207,
  RST0 = 208,
  RST1 = 209,
  RST2 = 210,
  RST3 = 211,
  RST4 = 212,
  RST5 = 213,
  RST6 = 214,
  RST7 = 215,
  SOI = 216,
  EOI = 217,
  SOS = 218,
  DQT = 219,
  DNL = 220,
  DRI = 221,
  DHP = 222,
  EXP = 223,
  APP0 = 224,
  APP1 = 225,
  APP2 = 226,
  APP3 = 227,
  APP4 = 228,
  APP5 = 229,
  APP6 = 230,
  APP7 = 231,
  APP8 = 232,
  APP9 = 233,
  APP10 = 234,
  APP11 = 235,
  APP12 = 236,
  APP13 = 237,
  APP14 = 238,
  APP15 = 239,
  JPG1 = 240,
  JPG2 = 241,
  JPG3 = 242,
  JPG4 = 243,
  JPG5 = 244,
  JPG6 = 245,
  JPG7 = 246,
  JPG8 = 247,
  JPG9 = 248,
  JPG10 = 249,
  JPG11 = 250,
  JPG12 = 251,
  JPG13 = 252,
  JPG14 = 253,
  COM = 254
} jpeg_segment_id_t;

/* ************************* jpeg_load_from_stream ************************* */

int
jpeg_load_from_stream (const uint8_t *stream, size_t size,
                       jpeg_header_t *header, void **data)
{
  size_t pos = 0;
  uint16_t marker;
  size_t size_tmp;

  jpeg_context_t *context = jpeg_init ();
  if (context == NULL)
    {
      JPEG_LOG ("[E] Can not init jpeg loader context\n");
      return JPEG_ERROR_INIT;
    }

  *data = NULL; // Set pointer to NULL for safety
  header->components = NULL;
  if (size < 2)
    {
      JPEG_LOG ("[E] jpegloader: size of JPEG is too small\n");
      jpeg_free_context (context);
      return JPEG_ERROR_FILE_NOT_JPEG;
    }

  // Check for SOF
  marker = ((uint16_t)stream[pos] << 8) | stream[pos + 1];
  pos += 2;
  if (((marker & 0xFF00) != 0xFF00) || ((marker & 0xFF) != SOI))
    {
      JPEG_LOG ("[E] jpegloader: SOI expected but 0x%04hx found\n",
                marker & 0xFF);
      jpeg_free_context (context);
      return JPEG_ERROR_FILE_NOT_JPEG;
    }
  else
    {
      DEBUG_LOG ("[D] jpegloader: SOI found\n");
    }

  // NOTE : No need cleanup cause no malloc was called

  while (true)
    {
      // Get marker
      if (pos < (size - 1))
        {
          marker = ((uint16_t)stream[pos] << 8) | stream[pos + 1];
          pos += 2;
          DEBUG_LOG ("[D] jpegloader: Copy marker 0x%04hx\n", marker);
        }
      else
        {
          DEBUG_LOG ("[E] jpegloader: End of stream\n");
          jpeg_free_context (context);
          return JPEG_ERROR_FILE_NOT_JPEG;
          break;
        }

      if ((marker & 0xFF00) != 0xFF00)
        {
          JPEG_LOG ("[E] jpegloader: Marker 0xFFXX expected\n");
          jpeg_free_context (context);
          return JPEG_ERROR_FILE_NOT_JPEG;
        }

      if ((marker & 0xFF) == EOI)
        {
          DEBUG_LOG ("[D] jpegloader: EOI found\n");
          break;
        }

      switch (marker & 0xFF)
        {
        case APP0: // APPn
        case APP1:
        case APP2:
        case APP3:
        case APP4:
        case APP5:
        case APP6:
        case APP7:
        case APP8:
        case APP9:
        case APP10:
        case APP11:
        case APP12:
        case APP13:
        case APP14:
        case APP15:
          DEBUG_LOG ("[D] jpegloader: APPn found\n");
          jpeg_app_extract (context, (marker & 0xFF) - APP0, stream, size,
                            &pos);
          break;

        case COM: // COM
          DEBUG_LOG ("[D] jpegloader: COM found\n");
          if (!jpeg_com_extract (context, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: COM parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case DAC: // DAC
          DEBUG_LOG ("[D] jpegloader: DAC found\n");
          // jpeg_dac_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case DHP: // DHP
          DEBUG_LOG ("[D] jpegloader: DHP found\n");
          // jpeg_dhp_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case DHT: // DHT
          DEBUG_LOG ("[D] jpegloader: DHT found\n");
          if (!jpeg_dht_extract (context, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: DHT parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case DNL: // DNL
          DEBUG_LOG ("[D] jpegloader: DNL found\n");
          // jpeg_dnl_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case DQT: // DQT
          DEBUG_LOG ("[D] jpegloader: DQT found\n");
          if (!jpeg_dqt_extract (context, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: DQT parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case DRI: // DRI
          DEBUG_LOG ("[D] jpegloader: DRI found\n");
          // jpeg_dri_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case EXP: // EXP
          DEBUG_LOG ("[D] jpegloader: EXP found\n");
          // jpeg_exp_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case JPG0: // JPGn
        case JPG1:
        case JPG2:
        case JPG3:
        case JPG4:
        case JPG5:
        case JPG6:
        case JPG7:
        case JPG8:
        case JPG9:
        case JPG10:
        case JPG11:
        case JPG12:
        case JPG13:
        case JPG14:
          DEBUG_LOG ("[D] jpegloader: JPGn found\n");
          // jpeg_jpg_extract (context, number, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case RST0: // RSTn
        case RST1:
        case RST2:
        case RST3:
        case RST4:
        case RST5:
        case RST6:
        case RST7:
          DEBUG_LOG ("[D] jpegloader: RSTn found\n");
          // jpeg_rst_extract (context, number, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        case SOF0: // SOF0
          DEBUG_LOG ("[D] jpegloader: SOF0 found\n");
          if (!jpeg_sof_extract (context, 0, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF0 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF1: // SOF1
          DEBUG_LOG ("[D] jpegloader: SOF1 found\n");
          if (!jpeg_sof_extract (context, 1, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF1 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF2: // SOF2
          DEBUG_LOG ("[D] jpegloader: SOF2 found\n");
          if (!jpeg_sof_extract (context, 2, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF2 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF3: // SOF3
          DEBUG_LOG ("[D] jpegloader: SOF3 found\n");
          if (!jpeg_sof_extract (context, 3, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF3 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF4: // SOF4
          DEBUG_LOG ("[D] jpegloader: SOF4 found\n");
          if (!jpeg_sof_extract (context, 4, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF4 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF5: // SOF5
          DEBUG_LOG ("[D] jpegloader: SOF5 found\n");
          if (!jpeg_sof_extract (context, 5, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF5 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF6: // SOF6
          DEBUG_LOG ("[D] jpegloader: SOF6 found\n");
          if (!jpeg_sof_extract (context, 6, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF6 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF7: // SOF7
          DEBUG_LOG ("[D] jpegloader: SOF7 found\n");
          if (!jpeg_sof_extract (context, 7, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF7 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF8: // SOF8
          DEBUG_LOG ("[D] jpegloader: SOF8 found\n");
          if (!jpeg_sof_extract (context, 8, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF8 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF9: // SOF9
          DEBUG_LOG ("[D] jpegloader: SOF9 found\n");
          if (!jpeg_sof_extract (context, 9, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF9 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF10: // SOF10
          DEBUG_LOG ("[D] jpegloader: SOF10 found\n");
          if (!jpeg_sof_extract (context, 10, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF10 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF11: // SOF11
          DEBUG_LOG ("[D] jpegloader: SOF11 found\n");
          if (!jpeg_sof_extract (context, 11, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF11 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOF12: // SOF12
          DEBUG_LOG ("[D] jpegloader: SOF12 found\n");
          if (!jpeg_sof_extract (context, 12, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOF12 parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case SOS:
          DEBUG_LOG ("[D] jpegloader: SOS found\n");
          if (!jpeg_sos_extract (context, stream, size, &pos))
            {
              JPEG_LOG ("[E] jpegloader: SOS parse error\n");
              jpeg_free_context (context);
              return JPEG_ERROR_FILE_NOT_JPEG;
            }
          break;

        case TEM: // TEM
          DEBUG_LOG ("[D] jpegloader: TEM found\n");
          // jpeg_tem_extract (context, stream, size, &pos);
          DEBUG_LOG ("[D] jpegloader: Not realised yet\n");
          jpeg_free_context (context);
          return JPEG_ERROR_NOT_SUPPORTED;
          break;

        default:
          JPEG_LOG ("[E] jpegloader: Unknown marker\n");
          jpeg_free_context (context);
          return JPEG_ERROR_FILE_NOT_JPEG;
        }
    }

  // Check for consistency
  // Check for SOF
  DEBUG_LOG ("[D] jpegloader: SOF%hhi.P = %hhi\n", context->header.header_n,
             context->header.precision);
  if (context->header.header_n != 0)
    {
      JPEG_LOG ("[E] jpegloader: Only baseline jpeg is supported\n");
      jpeg_free_context (context);
      return JPEG_ERROR_NOT_SUPPORTED;
    }
  if (context->header.precision != 8)
    {
      JPEG_LOG (
          "[E] jpegloader: only 8-bit precision is supported for Baseline\n");
      jpeg_free_context (context);
      return JPEG_ERROR_NOT_SUPPORTED;
    }
  DEBUG_LOG ("[D] jpegloader: SOF%hhi.Y = %hi\n", context->header.header_n,
             context->header.height);
  DEBUG_LOG ("[D] jpegloader: SOF%hhi.X = %hi\n", context->header.header_n,
             context->header.width);
  DEBUG_LOG ("[D] jpegloader: SOF%hhi.Nf = %hhi\n", context->header.header_n,
             context->header.n_component);
  for (size_t i = 0; i < context->header.n_component; i++)
    {
      DEBUG_LOG ("[D] jpegloader: SOF%hhi.C%i = %hhi\n",
                 context->header.header_n, (int)i,
                 context->header.components[i].component_id);
      DEBUG_LOG ("[D] jpegloader: SOF%hhi.H%i = %hhi\n",
                 context->header.header_n, (int)i,
                 SAMPLING_H (context->header.components[i].sampling));
      DEBUG_LOG ("[D] jpegloader: SOF%hhi.V%i = %hhi\n",
                 context->header.header_n, (int)i,
                 SAMPLING_V (context->header.components[i].sampling));
      DEBUG_LOG ("[D] jpegloader: SOF%hhi.Tq%i = %hhi\n",
                 context->header.header_n, (int)i,
                 context->header.components[i].dqt_destination);
    }
  for (size_t n = 0; n < 4; n++)
    {
      if (DQT_PRECISION (context->dqt[n].prec_dest) != DQT_DESTINATION_INV)
        {
          if (DQT_PRECISION (context->dqt[n].prec_dest) != 0)
            {
              JPEG_LOG ("[E] jpegloader: only 8-bit precision DQT is "
                        "supported for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: DQT%i.Pq = %hhi\n", (int)n,
                     DQT_PRECISION (context->dqt[n].prec_dest));
          DEBUG_LOG ("[D] jpegloader: DQT%i.Tq = %hhi\n", (int)n,
                     DQT_DESTINATION (context->dqt[n].prec_dest));
        }
    }

  size_tmp = 0;
  for (size_t n = 0; n < 4; n++)
    {
      if (DHT_CLASS (context->dht_dc[n].class_dest) != DHT_CLASS_INV)
        {
          DEBUG_LOG ("[D] jpegloader: DC DHT%zi.Tc = %hhi\n", n,
                     DHT_CLASS (context->dht_dc[n].class_dest));
          if (DHT_DESTINATION (context->dht_dc[n].class_dest) > 1)
            {
              JPEG_LOG ("[E] jpegloader: only 0 or 1 DHT destination is "
                        "supported for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: DC DHT%zi.Th = %hhi\n", n,
                     DHT_DESTINATION (context->dht_dc[n].class_dest));
          size_tmp++;
        }
    }
  for (size_t n = 0; n < 4; n++)
    {
      if (DHT_CLASS (context->dht_ac[n].class_dest) != DHT_CLASS_INV)
        {
          DEBUG_LOG ("[D] jpegloader: AC DHT%zi.Tc = %hhi\n", n,
                     DHT_CLASS (context->dht_ac[n].class_dest));
          if (DHT_DESTINATION (context->dht_ac[n].class_dest) > 1)
            {
              JPEG_LOG ("[E] jpegloader: only 0 or 1 DHT destination is "
                        "supported for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: AC DHT%zi.Th = %hhi\n", n,
                     DHT_DESTINATION (context->dht_ac[n].class_dest));
          size_tmp++;
        }
    }

  uint8_t u8;
  if (context->scans_count != 0)
    {
      for (int i = 0; i < context->scans_count; i++)
        {
          DEBUG_LOG ("[D] jpegloader: SOS%i.Ns = %hhi\n", i,
                     context->scans[i].n_component);
          for (int j = 0; j < context->scans[i].n_component; j++)
            {
              DEBUG_LOG ("[D] jpegloader: SOS%i.Cs%i = %hhi\n", i, j,
                         context->scans[i].components[j].component_selector);

              u8 = SOS_DC (context->scans[i].components[j].scan_destination);
              if (DHT_DESTINATION (context->dht_dc[u8].class_dest) != u8)
                {
                  JPEG_LOG ("[W] jpegloader: SOS%i.Td%i is set to %hhi, but "
                            "no such DC DHT was found. Copy from DC DHT 0\n",
                            i, j, u8);
                  if (!jpeg_dht_copy (context, context->dht_dc, DHT_CLASS_DC,
                                      u8))
                    {
                      JPEG_LOG ("[E] jpegloader: DHT copying failed\n");
                      return JPEG_ERROR_INIT;
                    }
                }
              DEBUG_LOG (
                  "[D] jpegloader: SOS%i.Td%i = %hhi\n", i, j,
                  SOS_DC (context->scans[i].components[j].scan_destination));

              u8 = SOS_AC (context->scans[i].components[j].scan_destination);
              if (DHT_DESTINATION (context->dht_ac[u8].class_dest) != u8)
                {
                  JPEG_LOG ("[W] jpegloader: SOS%i.Ta%i is set to %hhi, but "
                            "no such AC DHT was found. Copy from DC DHT 0\n",
                            i, j, u8);
                  if (!jpeg_dht_copy (context, context->dht_dc, DHT_CLASS_AC,
                                      u8))
                    {
                      JPEG_LOG ("[E] jpegloader: DHT copying failed\n");
                      return JPEG_ERROR_INIT;
                    }
                }
              DEBUG_LOG (
                  "[D] jpegloader: SOS%i.Ta%i = %hhi\n", i, j,
                  SOS_AC (context->scans[i].components[j].scan_destination));
            }

          if (context->scans[i].start != 0)
            {
              JPEG_LOG (
                  "[E] jpegloader: start predictor must be 0 for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: SOS%i.Ss = %hhi\n", i,
                     SOS_AC (context->scans[i].start));

          if (context->scans[i].end != 63)
            {
              JPEG_LOG (
                  "[E] jpegloader: end predictor must be 63 for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: SOS%i.Se = %hhi\n", i,
                     SOS_AC (context->scans[i].end));

          if (SOS_APPROX_H (context->scans[i].approximation) != 0)
            {
              JPEG_LOG ("[E] jpegloader: high approximation bit position must "
                        "be 0 for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: SOS%i.Ah = %hhi\n", i,
                     SOS_APPROX_H (context->scans[i].approximation));

          if (SOS_APPROX_L (context->scans[i].approximation) != 0)
            {
              JPEG_LOG ("[E] jpegloader: low approximation bit position must "
                        "be 0 for Baseline\n");
              jpeg_free_context (context);
              return JPEG_ERROR_NOT_SUPPORTED;
            }
          DEBUG_LOG ("[D] jpegloader: SOS%i.Al = %hhi\n", i,
                     SOS_APPROX_L (context->scans[i].approximation));
        }
    }
  else
    {
      JPEG_LOG ("[E] jpegloader: file has no scans\n");
      jpeg_free_context (context);
      return JPEG_ERROR_FILE_NOT_JPEG;
    }

  // Decoding
  jpeg_error_t err;
  if ((err = jpeg_decoding (context, header, data)) != JPEG_NO_ERROR)
    {
      JPEG_LOG ("[E] jpegloader: decoding error\n");
      jpeg_free_context (context);
      return err;
    }

  jpeg_free_context (context);
  return JPEG_NO_ERROR;
}

/* ************************** jpeg_load_from_file ************************** */

int
jpeg_load_from_file (const char *path, jpeg_header_t *header, void **data)
{
  FILE *file = fopen (path, "rb");
  if (file == NULL)
    {
      JPEG_LOG ("[E] jpegloader: Can not open file \"%s\"\n", path);
      return JPEG_ERROR_FILE_OPEN;
    }

  fseek (file, 0, SEEK_END);
  size_t sz = ftell (file);
  fseek (file, 0, SEEK_SET);

  if (sz == 0)
    {
      JPEG_LOG ("[E] jpegloader: File \"%s\" is empty\n", path);
      return JPEG_ERROR_FILE_NOT_JPEG;
    }

  uint8_t *stream = (uint8_t *)malloc (sz);
  if (stream == NULL)
    {
      JPEG_LOG ("[E] jpegloader: Can not allocate memory");
      fclose (file);
      return JPEG_ERROR_MALLOC;
    }

  fread (stream, sz, 1, file);
  fclose (file);

  return jpeg_load_from_stream (stream, sz, header, data);
}

/* ******************************* jpeg_free ******************************* */

void
jpeg_free (void **data, jpeg_header_t *header)
{
  if (*data != NULL)
    {
      free (*data);
      *data = NULL;
    }
  if (header != NULL)
    {
      if (header->components != NULL)
        {
          free (header->components);
          header->components = NULL;
          header->n_component = 0;
        }
    }
}

/* ************************** jpeg_loader_version ************************** */

void
jpeg_loader_version (char *version_string, size_t string_size)
{
  strncpy (version_string, VERSION, string_size);
}
