#include "huffman.h"
#include "jpeglog.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

huffman_node_t *h_make_node (uint8_t length, int value, huffman_node_t *root,
                             bool *status);
void h_print (FILE *where, int value, uint8_t length);

/* ************************** h_create_from_codes ************************** */

huffman_node_t *
h_create_from_codes (jpeg_dht_t *table)
{
  huffman_node_t *tree;
  bool status;

  if (table == NULL)
    return NULL;

  tree = (huffman_node_t *)malloc (sizeof (huffman_node_t));
  if (tree == NULL)
    return NULL;
  tree->left = NULL;
  tree->right = NULL;
  tree->value = -1;
  tree->code = 0;
  tree->level = 0;

  for (int i = 1; i < 17; i++)
    {
      if (table->lengths[i - 1] != 0)
        {
          for (int j = 0; j < table->lengths[i - 1]; j++)
            {
              status = false;
              if (h_make_node (i, table->values[i - 1][j], tree, &status)
                  == NULL)
                {
                  JPEG_LOG ("[E] jpegloader: Huffman tree construct error\n");
                  return NULL;
                }
            }
        }
    }

  return tree;
}

/* ****************************** h_clear_tree ***************************** */

void
h_clear_tree (huffman_node_t *root)
{
  if (root == NULL)
    {
      return;
    }
  
  h_clear_tree (root->left);
  h_clear_tree (root->right);

  free (root);
}

/* ********************************* h_move ******************************** */

huffman_node_t *
h_move (const huffman_node_t *from, uint32_t val)
{
  if (from == NULL)
    return NULL;

  if (val == 0)
    {
      return from->left;
    }
  else
    {
      return from->right;
    }
}

/* ******************************* h_is_leaf ******************************* */

bool
h_is_leaf (const huffman_node_t *node)
{
  if (node == NULL)
    return false;
  else
    return (node->value >= 0);
}

/* ****************************** h_make_node ****************************** */

huffman_node_t *
h_make_node (uint8_t length, int value, huffman_node_t *root, bool *status)
{
  huffman_node_t *node;

  if (root == NULL || (*status))
    return NULL;

  if (length == 0)
    {
      root->value = value;
      return root;
    }

  // Left node is not exist. When created it will be empty
  if (root->left == NULL)
    {
      root->left = (huffman_node_t *)malloc (sizeof (huffman_node_t));
      if (root->left == NULL)
        {
          *status = true;
          JPEG_LOG ("[E] Can not allocate memory\n");
          return NULL;
        }

      root->left->left = NULL;
      root->left->right = NULL;
      root->left->value = -1;
      root->left->level = root->level + 1;
      root->left->code = (root->code << 1);
      node = h_make_node (length - 1, value, root->left, status);
      return node;
    }
  else
    {
      if (!h_is_leaf (root->left)) // Left exists and not leaf
        {
          node = h_make_node (length - 1, value, root->left, status);
          if (node == NULL) // Node is busy check right
            {
              if (root->right == NULL)
                {
                  root->right
                      = (huffman_node_t *)malloc (sizeof (huffman_node_t));
                  if (root->right == NULL)
                    {
                      *status = true;
                      JPEG_LOG ("[E] Can not allocate memory\n");
                      return NULL;
                    }

                  root->right->left = NULL;
                  root->right->right = NULL;
                  root->right->value = -1;
                  root->right->level = root->level + 1;
                  root->right->code = (root->code << 1) + 1;
                  node = h_make_node (length - 1, value, root->right, status);
                  return node;
                }
              else
                {
                  if (!h_is_leaf (root->right)) // Right exists and not leaf
                    {
                      node = h_make_node (length - 1, value, root->right,
                                          status);
                      return node;
                    }
                  else
                    {
                      return NULL;
                    }
                }
            }
          else
            return node;
        }
      else
        {
          if (root->right == NULL)
            {
              root->right = (huffman_node_t *)malloc (sizeof (huffman_node_t));
              if (root->right == NULL)
                {
                  *status = true;
                  JPEG_LOG ("[E] Can not allocate memory\n");
                  return NULL;
                }

              root->right->left = NULL;
              root->right->right = NULL;
              root->right->value = -1;
              root->right->level = root->level + 1;
              root->right->code = (root->code << 1) + 1;
              node = h_make_node (length - 1, value, root->right, status);
              return node;
            }
          else
            {
              if (!h_is_leaf (root->right)) // Right exists and not leaf
                {
                  node = h_make_node (length - 1, value, root->right, status);
                  return node;
                }
              else
                {
                  return NULL;
                }
            }
        }
    }

  return node;
}

/* ******************************** h_print ********************************
 */

void
h_print (FILE *where, int value, uint8_t length)
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

/* ****************************** h_print_node *****************************
 */

void
h_print_node (FILE *where, const huffman_node_t *node)
{
  if (node == NULL)
    {
      fprintf (where, "NULL\n");
      return;
    }
  else if (node->value >= 0)
    {
      fprintf (where, "Leaf (");
      h_print (where, node->code, node->level);
      fprintf (where, ")\n");
    }

  h_print_node (where, node->left);
  h_print_node (where, node->right);
}
