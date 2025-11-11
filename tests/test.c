#include "jpegloader/jpegtypes.h"
#include <jpegloader/jpegloader.h>

#include <stdio.h>
#include <string.h>

#define PACKAGE "test"

#define VERSION "0.1"

/* ********************************* usage ********************************* */

void
usage ()
{
  printf ("Usage: %s [ARGS] <FILE>\n\n", PACKAGE);
  printf ("Arguments:\n");
  printf ("  <FILE>               Input JPEG file\n\n");
  printf ("Options:\n");
  printf ("  --help, -h           Show this message and exit\n");
  printf ("  --version, -v        Show version prompt and exit\n");
}

/* ******************************** version ******************************** */

void
version ()
{
  printf ("%s %s\n", PACKAGE, VERSION);
  printf ("Copyright (C) 2025 Dmitry Khruschev aka DimaO\n");
  printf ("License GPLv3+: GNU GPL version 3 or later "
          "<https://gnu.org/licenses/gpl.html>\n");
  printf (
      "This is free software: you are free to change and redistribute it.\n");
  printf ("There is NO WARRANTY, to the extent permitted by law.\n");
}

/* ********************************** main ********************************* */

int
main (int argc, char **argv)
{
  char path[256];

  if (argc < 2)
    {
      usage ();
      return 0;
    }
  else
    {
      for (int i = 1; i < argc; i++)
        {
          if ((strcmp (argv[i], "-h") == 0)
              || (strcmp (argv[i], "--help") == 0))
            {
              usage ();
              return 0;
            }
          else if ((strcmp (argv[i], "-v") == 0)
                   || (strcmp (argv[i], "--version") == 0))
            {
              version ();
              return 0;
            }
          else if (argv[i][0] == '-')
            {
              printf ("[E] Unknown option \"%s\"\n", argv[i]);
            }
          else
            {
              strncpy (path, argv[i], 256);
            }
        }

      jpeg_header_t hdr;
      void *data;
      jpeg_error_t err = jpeg_load_from_file (path, &hdr, &data);
      switch (err)
        {
        case JPEG_NO_ERROR:
          break;

        case JPEG_ERROR_FILE_OPEN:
          printf ("[E] File \"%s\" can not be read\n", path);
          fflush (stdout);
          break;

        case JPEG_ERROR_FILE_NOT_JPEG:
          printf ("[E] File \"%s\" is not a jpeg\n", path);
          fflush (stdout);
          break;

        case JPEG_ERROR_MALLOC:
          printf ("[E] Can not allocate memory\n");
          fflush (stdout);
          break;

        case JPEG_ERROR_NOT_SUPPORTED:
          printf ("[E] Not supported\n");
          fflush (stdout);
          break;

        default:
          printf ("[E] Unknown error code (%i)\n", (int)err);
          fflush (stdout);
          break;
        }
      jpeg_free (&data, &hdr);
      return (err == JPEG_NO_ERROR) ? 0 : -1;
    }
}
