/**
 * \file
 * \brief Huffman tree module
 *
 * This file defines the huffman_node_t Huffman tree
 * descriptor and all the functions to work with it.
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "jpegcontext.h"

#include <stdio.h>

#define H_CATEGORY_POS 0
#define H_RLE_POS 4
#define H_CATEGORY(x) ((x >> H_CATEGORY_POS) & 0xF)
#define H_RLE(x) ((x >> H_RLE_POS) & 0xF)

/**
 * \brief Create Huffman tree
 * \param [in] table -- Huffman table (JPEG)
 * \return Returns complete Huffman tree on success or NULL on error.
 */
huffman_node_t *h_create_from_codes (jpeg_dht_t *table);

void h_clear_tree (huffman_node_t *root);

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
void h_print (FILE *where, int value, uint8_t length);

void h_print_node (FILE *where, const huffman_node_t *node);

/**
 * \brief Move from node \c from to next node by val code
 * \param [in] from  -- Initial node
 * \param [in] value -- Next bit in Huffman code (0 means 0, others means 1)
 * \return Returns Huffman tree node for \c val step or NULL if node is
 * missing.
 */
huffman_node_t *h_move (const huffman_node_t *from, uint32_t val);

/**
 * \brief Check if node is leaf (has no children)
 * \param [in] node -- Checked node
 * \return Reurns \c true if node is leaf otherwise returns \c false .
 */
bool h_is_leaf (const huffman_node_t *node);

#endif // HUFFMAN_H