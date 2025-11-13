/**
 * \file
 * \brief Huffman tree module
 *
 * This file defines the @ref huffman_node_t Huffman tree
 * descriptor and all the functions to work with it.
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "jpegcontext.h"

#include <stdio.h>

/**
 * \brief Create Huffman tree
 * \param [in] table -- Huffman table (JPEG)
 * \return Returns complete Huffman tree on success or NULL on error.
 */
huffman_node_t *h_create_from_codes (jpeg_dht_t *table);

/**
 * \brief Clear Huffman tree
 * \param [in] tree -- Huffman tree
 */
void h_clear (huffman_node_t *tree);

/**
 * \brief Print Huffman code in binary form
 * \param [in] where  -- IO stream for output
 * \param [in] value  -- Huffman code
 * \param [in] length -- Length of the Huffman code
 */
void h_print (FILE *where, uint16_t value, uint8_t length);

#endif // HUFFMAN_H