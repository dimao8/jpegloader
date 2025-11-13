#include "huffman.h"
#include "jpeglog.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

huffman_node_t *h_make_node (uint16_t code, uint8_t length, uint16_t value,
                             huffman_node_t *root);
void h_print (FILE *where, uint16_t value, uint8_t length);
void h_print_node (FILE *where, const huffman_node_t *node, int level);

/* ************************** h_create_from_codes ************************** */

huffman_node_t *
h_create_from_codes (jpeg_dht_t *table)
{
  huffman_node_t *tree;
  uint16_t value = 0;

  if (table == NULL)
    return NULL;

  tree = (huffman_node_t *)malloc (sizeof (huffman_node_t));
  if (tree == NULL)
    return NULL;
  tree->left = NULL;
  tree->right = NULL;
  tree->unused = true;

  for (int i = 1; i < 16; i++)
    {
      for (uint16_t j = 0; j < table->lengths[i - 1]; j++)
        {
          h_make_node (value, i, table->values[i - 1][j], tree);

          value++;
        }
      value <<= 1;
    }

  DEBUG_LOG ("[D] ");
#ifndef NDEBUG
  h_print_node (stdout, tree, 0);
#endif // NDEBUG

  return tree;
}

/* ****************************** h_make_node ****************************** */

huffman_node_t *
h_make_node (uint16_t code, uint8_t length, uint16_t value,
             huffman_node_t *root)
{
  if (root == NULL || length == 0)
    return NULL;

  uint16_t mask = 1 << (length - 1);
  huffman_node_t *node = root;

  for (int i = 0; i < length; i++)
    {
      if (code & mask) // Right
        {
          if (node->right == NULL)
            {
              node->right = (huffman_node_t *)malloc (sizeof (huffman_node_t));
              if (node->right == NULL)
                return NULL;
              node->right->right = NULL;
              node->right->left = NULL;
              node->right->unused = true;
            }
          node = node->right;
        }
      else // Left
        {
          if (node->left == NULL)
            {
              node->left = (huffman_node_t *)malloc (sizeof (huffman_node_t));
              if (node->left == NULL)
                return NULL;
              node->left->right = NULL;
              node->left->left = NULL;
              node->left->unused = true;
            }
          node = node->left;
        }
      code <<= 1;
    }

  node->value = value;

  return node;
}

/* ******************************** h_print ******************************** */

void
h_print (FILE *where, uint16_t value, uint8_t length)
{
  if (length > 16 || length == 0)
    return;

  uint16_t mask = 1 << (length - 1);

  for (size_t i = 0; i < length; i++)
    {
      fputc ((value & mask) ? '1' : '0', where);
      value <<= 1;
    }
}

/* ****************************** h_print_node ***************************** */

void
h_print_node (FILE *where, const huffman_node_t *node, int level)
{
  if (node == NULL)
    printf ("NULL\n");
  else
    {
      for (int i = 0; i < level; i++)
        {
          fputc (' ', where);
          fputc (' ', where);
        }
      if (node->left == NULL && node->right == NULL)
        fprintf (where, "0x%hu\n", node->value);
      else
        fprintf (where, "branch\n");
      printf ("left: ");
      h_print_node (where, node->left, level + 1);
      printf ("right: ");
      h_print_node (where, node->right, level + 1);
    }
}
