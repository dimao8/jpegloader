#include "huffman.h"
#include "jpeglog.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

huffman_node_t *h_make_node (uint8_t length, int value, huffman_node_t *root);
void h_print (FILE *where, int value, uint8_t length);

/* ************************** h_create_from_codes ************************** */

huffman_node_t *
h_create_from_codes (jpeg_dht_t *table)
{
  huffman_node_t *tree;

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
              // DEBUG_LOG ("[D] Create new Huffman code for length = %i\n", i);
              if (h_make_node (i, table->values[i - 1][j], tree) == NULL)
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
    return;

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
      // DEBUG_LOG ("[D] Go to the left\n");
      return from->left;
    }
  else
    {
      // DEBUG_LOG ("[D] Go to the right\n");
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
h_make_node (uint8_t length, int value, huffman_node_t *root)
{
  huffman_node_t *node;

  if (root == NULL)
    return NULL;

  // DEBUG_LOG ("[D] h_make_node (%hhu, %i, (huffman)(", length, value);
  // h_print (stdout, root->code, root->level);
  // DEBUG_LOG ("))\n");

  if (length == 0)
    {
      // DEBUG_LOG ("[D] Set value %i in (huffman)(", value);
      // h_print (stdout, root->code, root->level);
      // DEBUG_LOG (")\n");
      root->value = value;
      return root;
    }
  else
    {
      // DEBUG_LOG ("[D] Try to go left\n");
      // Left node is not exist. When created it will be empty
      if (root->left == NULL)
        {
          // DEBUG_LOG ("[D] Left is not exist. Create\n");
          root->left = (huffman_node_t *)malloc (sizeof (huffman_node_t));
          if (root->left == NULL)
            {
              JPEG_LOG ("[E] Can not allocate memory\n");
              return NULL;
            }

          root->left->left = NULL;
          root->left->right = NULL;
          root->left->value = -1;
          root->left->level = root->level + 1;
          root->left->code = (root->code << 1);
          node = h_make_node (length - 1, value, root->left);
          if (node == NULL) // Error
            {
              // DEBUG_LOG ("[D] Left was created but node is not set\n");
              return NULL;
            }
          else
            return node;
        }
      else
        {
          // DEBUG_LOG ("[D] Left node exists. Check for leaf\n");
          if (!h_is_leaf (root->left)) // Left exists and not leaf
            {
              // DEBUG_LOG ("[D] Left node is not a leaf\n");
              node = h_make_node (length - 1, value, root->left);
              if (node == NULL) // Node is busy check right
                {
                  // DEBUG_LOG ("[D] Left node is busy. Check right\n");
                  if (root->right == NULL)
                    {
                      // DEBUG_LOG ("[D] Right is not exist. Create\n");
                      root->right
                          = (huffman_node_t *)malloc (sizeof (huffman_node_t));
                      if (root->right == NULL)
                        {
                          JPEG_LOG ("[E] Can not allocate memory\n");
                          return NULL;
                        }

                      root->right->left = NULL;
                      root->right->right = NULL;
                      root->right->value = -1;
                      root->right->level = root->level + 1;
                      root->right->code = (root->code << 1) + 1;
                      node = h_make_node (length - 1, value, root->right);
                      if (node == NULL) // Error
                        {
                          // DEBUG_LOG (
                          //     "[D] Right was created but node is not set\n");
                          return NULL;
                        }
                      else
                        return node;
                    }
                  else
                    {
                      // DEBUG_LOG ("[D] Right node exists. Check for leaf\n");
                      if (!h_is_leaf (
                              root->right)) // Right exists and not leaf
                        {
                          // DEBUG_LOG ("[D] Right node is not a leaf\n");
                          node = h_make_node (length - 1, value, root->right);
                          if (node == NULL) // Error
                            {
                              // DEBUG_LOG ("[D] Right was created but node was "
                              //            "not set\n");
                              return NULL;
                            }
                          else
                            return node;
                        }
                      else
                        {
                          // DEBUG_LOG ("[D] Right node is a leaf. Going up\n");
                          return NULL;
                        }
                    }
                }
              else
                return node;
            }
          else
            {
              // DEBUG_LOG ("[D] Left node is a leaf. Check right\n");
              if (root->right == NULL)
                {
                  // DEBUG_LOG ("[D] Right is not exist. Create\n");
                  root->right
                      = (huffman_node_t *)malloc (sizeof (huffman_node_t));
                  if (root->right == NULL)
                    {
                      JPEG_LOG ("[E] Can not allocate memory\n");
                      return NULL;
                    }

                  root->right->left = NULL;
                  root->right->right = NULL;
                  root->right->value = -1;
                  root->right->level = root->level + 1;
                  root->right->code = (root->code << 1) + 1;
                  node = h_make_node (length - 1, value, root->right);
                  if (node == NULL) // Error
                    {
                      // DEBUG_LOG (
                      //     "[D] Right was created but node was not set\n");
                      return NULL;
                    }
                  else
                    return node;
                }
              else
                {
                  // DEBUG_LOG ("[D] Right node exists. Check for leaf\n");
                  if (!h_is_leaf (root->right)) // Right exists and not leaf
                    {
                      // DEBUG_LOG ("[D] Right node is not a leaf\n");
                      node = h_make_node (length - 1, value, root->right);
                      if (node == NULL) // Error
                        {
                          // DEBUG_LOG ("[D] Right and left both leaves\n");
                          return NULL;
                        }
                      else
                        return node;
                    }
                  else
                    {
                      // DEBUG_LOG ("[D] Right node is a leaf. Going up\n");
                      return NULL;
                    }
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
