#ifndef JPEGLOADER_H
#define JPEGLOADER_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#if defined(_WIN32)
#define EXPORT __declspec (dllexport)
#elif defined(__linux__)
#define EXPORT
#endif

#include <jpegloader/jpegtypes.h>

#include <stddef.h>
#include <stdint.h>

  /**
   * \brief Load jpeg image form stream
   * \param [in] stream  -- Memory stream (buffer) with jpeg image
   * \param [in] size    -- Size of the memory stream (buffer)
   * \param [out] header -- Output buffer for jpeg header
   * \param [out] data   -- Pointer to the output data buffer (double pointer)
   * \return Returns result of operation (see. \see jpeg_error_t)
   */
  int EXPORT jpeg_load_from_stream (const uint8_t *stream, size_t size,
                                    jpeg_header_t *header, void **data);

  /**
   * \brief Load jpeg image form stream
   * \param [in] path    -- Path to the jpeg file
   * \param [out] header -- Output buffer for jpeg header
   * \param [out] data   -- Pointer to the output data buffer (double pointer)
   * \return Returns result of operation (see. \see jpeg_error_t)
   */
  int EXPORT jpeg_load_from_file (const char *path, jpeg_header_t *header,
                                  void **data);

  /**
   * \brief Clear JPEG image data
   * \param [in, out] data -- Pointer to the buffer from jpeg_load_from_file or
   * jpeg_load_from_stream
   */
  void EXPORT jpeg_free (void **data, jpeg_header_t *header);

  /**
   * \brief Output jpegloader version in N.N.N.N
   * \param [out] version_string -- Version string buffer
   * \param [in] string_size     -- Size of the buffer (at least 12)
   */
  void EXPORT jpeg_loader_version (char *version_string, size_t string_size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // JPEGLOADER_H
