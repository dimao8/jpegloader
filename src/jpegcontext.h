#ifndef JPEGCONTEXT_H
#define JPEGCONTEXT_H

#include <jpegloader/jpegtypes.h>

#include <stddef.h>
#include <stdint.h>

typedef struct scan_component_descriptor_tag
{
  uint8_t component_selector; /// Scan component selector
  uint8_t scan_destination;   /// Entropy coding table destination selector
} scan_component_descriptor_t;

/**
 * \brief Scan header (SOS marker content)
 */
typedef struct jpeg_scan_header_tag
{
  uint8_t n_component; /// Number of image components in scan
  scan_component_descriptor_t components[4]; /// Components
  uint8_t start;         /// Start of spectral or predictor selection
  uint8_t end;           /// End of spectral selection
  uint8_t approximation; /// Successive approximation bit position
  size_t scan_length;    /// Length of the scan data
  uint8_t *scan_data;    /// Scan data
} jpeg_scan_header_t;

#define SOS_DC_POS 4
#define SOS_AC_POS 0
#define SOS_DC(x) ((x >> SOS_DC_POS) & 0xF)
#define SOS_AC(x) ((x >> SOS_AC_POS) & 0xF)

#define SOS_APPROX_H_POS 4
#define SOS_APPROX_L_POS 0
#define SOS_APPROX_H(x) ((x >> SOS_APPROX_H_POS) & 0xF)
#define SOS_APPROX_L(x) ((x >> SOS_APPROX_L_POS) & 0xF)

/**
 * \brief Quantization table (DQT marker content)
 */
typedef struct jpeg_dqt_tag
{
  uint8_t prec_dest;  /// Quantization and precision table element precision
  uint16_t table[64]; /// Quantization table element
} jpeg_dqt_t;

#define DQT_PRECISION_POS 4
#define DQT_DESTINATION_POS 0
#define DQT_PRECISION(x) ((x >> DQT_PRECISION_POS) & 0xF)
#define DQT_DESTINATION(x) ((x >> DQT_DESTINATION_POS) & 0xF)
#define DQT_PRECISION_INV 0xF
#define DQT_DESTINATION_INV 0xF

/**
 * \brief Huffman table (DHT marker content)
 */
typedef struct jpeg_dht_tag
{
  uint8_t class_dest;  /// Table class and destination
  uint8_t lengths[16]; /// Number of Huffman codes per length
  uint8_t *values[16]; /// Value associated with each Huffman code
} jpeg_dht_t;

#define DHT_CLASS_POS 4
#define DHT_DESTINATION_POS 0
#define DHT_CLASS(x) ((x >> DHT_CLASS_POS) & 0xF)
#define DHT_DESTINATION(x) ((x >> DHT_DESTINATION_POS) & 0xF)
#define DHT_CLASS_INV 0xF
#define DHT_DESTINATION_INV 0xF

/**
 * \brief Define arithmetic coding (DAC marker content)
 * \note Not used in Baseline DCT
 */
typedef struct jpeg_arithmetic_coding_tag
{
  uint8_t class;       /// Table class
  uint8_t destination; /// Arithmetic coding conditioning table destination
                       /// identifier
} jpeg_arithmetic_coding_t;

/**
 * \brief Define restart interval (DRI marker content)
 */
typedef struct jpeg_restart_tag
{
  uint16_t interval; /// Restart interval
} jpeg_restart_t;

/**
 * \brief Comment (COM marker content)
 */
typedef struct jpeg_comment_tag
{
  char *comment; /// Comment string
} jpeg_comment_t;

/**
 * \brief Application (APPX marker content)
 * \note Will be skipped
 */
typedef struct jpeg_application_tag
{
  uint16_t size; /// Size of data
  void *data;    /// Application data
} jpeg_application_t;

/**
 * \brief Define number of lines (DNL marker content)
 */
typedef struct jpeg_number_lines_tag
{
  uint16_t n_lines; /// Number of lines
} jpeg_number_lines_t;

typedef struct jpeg_component_descriptor_tag
{
  uint8_t component_index;
  uint8_t h_dc_table_index;
  uint8_t h_ac_table_index;
  uint8_t q_table_index;
  uint8_t h_number;
  uint8_t v_number;
} jpeg_component_descriptor_t;

typedef struct jpeg_block_buffer_tag
{
  int dct_data[64];
  int data[64];
  jpeg_component_descriptor_t descriptor;
} jpeg_block_buffer_t;

/**
 * \brief JPEG state context
 */
typedef struct jpeg_context_tag
{
  jpeg_error_t error;         /// JPEG error code
  jpeg_header_t header;       /// JPEG header (SOF marker content)
  jpeg_application_t app[16]; /// App array (APP markers content)
  jpeg_dqt_t dqt[4];          /// Quantization tables (DQT markers content)
  jpeg_dht_t dht_dc[4];       /// DC Huffman tables
  huffman_node_t *huffman_tree_dc[4]; /// DC Huffman trees
  jpeg_dht_t dht_ac[4];               /// AC Huffman tables
  huffman_node_t *huffman_tree_ac[4]; /// AC Huffman trees
  size_t scans_capacity;              /// Memory for scans
  size_t scans_count;                 /// Number of scans
  jpeg_scan_header_t *scans;          /// Scan table
  size_t n_buffers;                   /// Number of buffers in MCU
  jpeg_block_buffer_t *mcu_buffers;   /// JPEG decoder MCU buffer
  uint8_t *rgb;
} jpeg_context_t;

#endif // JPEGCONTEXT_H
