#ifndef BST_H
#define BST_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    BST_INSERT_SUCCESS,
    BST_INSERT_FAILURE,
    BST_INSERT_EQUAL,
    BST_REPLACE_TRUE ,
    BST_REPLACE_FALSE,
    BST_FREE_PAYLOAD_TRUE,
    BST_FREE_PAYLOAD_FALSE
} bst_status_t;

typedef enum
{
    BST_LT,
    BST_GT,
    BST_EQ
} bst_compare_t;

typedef enum
{
    BST_IN_ORDER,
    BST_POST_ORDER,
    BST_PRE_ORDER
} bst_traversal_t;

typedef struct node node_t;
typedef struct bst_node_payload_t node_payload_t; // I Typedef their structure to something internal

typedef struct bst
{
    node_t * root;
    bst_compare_t (* compare)(node_payload_t *, node_payload_t *);
    void (* free_payload)(node_payload_t *);
} bst_t;

bst_t * bst_init(bst_compare_t (* compare)(node_payload_t *, node_payload_t *), void (* free_payload)(node_payload_t *));
void bst_destroy(bst_t * bst, bst_status_t free_payload);
bst_status_t bst_insert(bst_t * bst, node_payload_t * payload, bst_status_t replace);

//TODO: All a null function or structure here
void bst_traversal(bst_t * bst, bst_traversal_t type, void (* callback)(node_payload_t *, void *), void * void_ptr);
void print_2d(bst_t * bst, void (* callback)(node_payload_t *));

#endif //BST_H

