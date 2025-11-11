#include "huffman.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

bool h_node_is_leaf (huffman_node_t *node);

/* ************************** h_create_from_codes ************************** */

huffman_node_t *
h_create_from_codes (jpeg_dht_t *table)
{
  huffman_node_t *tree;
  huffman_node_t *node;
  uint16_t value;

  if (table == NULL)
    return NULL;

  tree = (huffman_node_t *)malloc (sizeof (huffman_node_t));
  if (tree == NULL)
    return NULL;
  tree->left = NULL;
  tree->right = NULL;
  tree->unused = true;

  for (int i = 0; i < 16; i++)
    {
      
    }

  return tree;
}

/* ***************************** h_node_is_leaf **************************** */

bool
h_node_is_leaf (huffman_node_t *node)
{
  if (node == NULL)
    return false;
  else if ((node->right == NULL) && (node->left == NULL))
    return true;
  else
    return false;
}
