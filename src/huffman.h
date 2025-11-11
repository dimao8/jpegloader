#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "jpegcontext.h"

/**
 * \brief Create Huffman tree
 * \param [in] table -- Huffman table (JPEG)
 * \return Returns complete Huffman tree on success or NULL on error.
 */
huffman_node_t *h_create_from_codes (jpeg_dht_t* table);

/**
 * \brief Clear Huffman tree
 * \param [in] tree -- Huffman tree
 */
void h_clear (huffman_node_t* tree);

#endif // HUFFMAN_H