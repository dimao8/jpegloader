/**
 * \file
 */

#ifndef JPEGTYPES_H
#define JPEGTYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef struct header_component_descriptor_tag
{
  uint8_t component_id;    /// Component identifier
  uint8_t sampling;        /// Sampling factor
  uint8_t dqt_destination; /// Quantization table destination selector
} header_component_descriptor_t;

#define SAMPLING_H(x) ((x >> 4) & 0xF)
#define SAMPLING_V(x) (x & 0xF)

/**
 * \brief JPEG header (SOF marker content)
 */
typedef struct jpeg_header_tag
{
  uint8_t header_n;    /// Number of header (i.e. 0 means SOF0)
  uint8_t precision;   /// Sample precision
  uint16_t height;     /// Number of lines
  uint16_t width;      /// Number of samples per line
  uint8_t n_component; /// Number of image components in frame
  header_component_descriptor_t *components; /// Components
} jpeg_header_t;

/**
 * \brief JPEG loader error codes
 */
typedef enum jpeg_error_tag
{
  JPEG_NO_ERROR = 0,        ///< There is no error
  JPEG_ERROR_FILE_OPEN,     ///< File can not be opened or does not exist
  JPEG_ERROR_FILE_NOT_JPEG, ///< File is not a JPEG image or has internal error
  JPEG_ERROR_MALLOC, ///< Can not allocate memory during decoding process
  JPEG_ERROR_NOT_SUPPORTED, ///< JPEG file is valid but not supported
  JPEG_ERROR_INIT,   ///< Can not initialize some structures (not uset for now)
  JPEG_ERROR_HUFFMAN ///< Huffman table or bitstream error
} jpeg_error_t;

/**
 * \brief Huffman tree descriptor
 * \struct huffman_node_t
 */
typedef struct huffman_node_tag huffman_node_t;

/**
 * \private
 */
typedef struct huffman_node_tag
{
  huffman_node_t *left;
  huffman_node_t *right;
  bool unused;
  uint8_t value;
} huffman_node_t;

#endif // JPEGTYPES_H
