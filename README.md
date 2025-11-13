# JPEGLoader library #

## Introduction ##

This library provides an easy way to load JPEG images with Baseline encoding.
It was designed as part of a glTF 2.0 loader that requires PNG and JPEG images
to be loaded as textures.

The library is written in C and includes CMake configuration for building and
installation. Read the INSTALL.md file to learn how to build it yourself.

## Use of library ##

Two header files are located in the <code>jpegloader</code> directory of your
<code>include</code> directory. Include <code><jpegloader/jpegloader.h></code>
to use the library in your projects. Link the library using the
<code>-ljpegloader</code> option.

The JPEG image loader provides a simple interface for loading images. There are
two versions of loading functions:

```c
int jpeg_load_from_stream (const uint8_t *stream, size_t size, jpeg_header_t *header, void **data)
```

and

```c
int jpeg_load_from_file (const char *path, jpeg_header_t *header, void **data)
```

The first function loads a JPEG image from a memory stream, while the second
loads an image from a file. <code>stream</code> is a pointer to the image data
in memory with the specified <code>size</code>. <code>path</code> contains the
string with the path to the image file. These functions fill the
<code>header</code> structure with image size information and create an array
in memory to hold the raw image data. The loaded image has an RGB triplet
structure with 8 bits per channel. Its size is <code>header->width</code> by
<code>header->height</code> pixels. Note that <code>data</code> is a pointer to
a pointer.

Here is an example code fragment for loading a JPEG from the file "image.jpeg":

```c
uint8_t *image;
jpeg_header_t header;

jpeg_error_t err = jpeg_load_from_file ("image.jpeg", &header, &image);
if (err != JPEG_NO_ERROR)
  {
    // Error handling
  }
```

If loading succeeds, <code>err</code> will contain the code
<code>JPEG_NO_ERROR</code>, header will be filled with the appropriate JPEG
header data, and the <code>image</code> pointer will point to a valid RGB
triplet array.

When the image is no longer needed, call <code>jpeg_free</code>. Here is its
prototype:

```c
void jpeg_free (void **data, jpeg_header_t *header)
```

Pass the filled array pointer and header to this function to safely free the
memory.

You can check the library version using:

```c
void jpeg_loader_version (char *version_string, size_t string_size)
```

<code>version_string</code> will contain the version string in "N.N.N.N"
format. <code>version_string</code> must be large enough to hold the entire
version string plus the <code>null</code> character. <code>string_size</code>
must contain the total length of the string buffer.
