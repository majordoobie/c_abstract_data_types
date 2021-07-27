#include <bst.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

struct node {
    int height;
    struct node * parent;
    struct node * left_child;
    struct node * right_child;
    node_payload_t * key;
};

static node_t * find_max_payload(node_t * node);
static node_t * find_min_payload(node_t * node);
static node_t * remove_node(node_t * node, node_payload_t * payload, bst_t * bst);
static int get_height(node_t * node);
static node_t * insert_node(node_t * node, node_payload_t * payload, bst_status_t replace, bst_t * bst);
static int get_balance_factor(node_t * node);
static bool is_right_heavy(node_t * node);
static bool is_left_heavy(node_t * node);
static node_t * balance_tree(node_t * node, bst_t * bst);
static void set_height(node_t * node);

static node_t * right_rotation(node_t * node, bst_t * bst);
static node_t * left_rotation(node_t *node, bst_t * bst);

static void free_node(bst_t * bst, node_t * node, bst_status_t free_payload);
static void traversal_free(bst_t * bst, node_t * node, bst_status_t free_payload);
static void print_2d_iter(node_t * node, int space, void (* callback)(node_payload_t *));
static void in_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr);
static void pre_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr);
static void post_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr);
static node_t * search_node(bst_t * bst, node_t * node, node_payload_t * target_payload);


/*!
 * @brief Public function used to printout the binary tree in a way that can visualized
 * @param bst[in] bst_t
 * @param callback[in] Call back function used to call on each node. Ideally, this is for
 * printint each node
 */
void print_2d(bst_t * bst, void (*callback)(node_payload_t *))
{
    print_2d_iter(bst->root, 0, callback);
}

/*!
 * @brief Initialize the tree structure to include the payload size of the node keys.
 * The bst structure also contains the compare function pointer performed on each node
 * that is passed around.
 * @param payload_size[in] The size of each node->key payload
 * @param compare[in] Pointer to a compare function that is performed on each node
 * @return Pointer to the BST structure with the bst->root set to NULL
 */
bst_t * bst_init(bst_compare_t (* compare)(node_payload_t *, node_payload_t *), void (* free_payload)(node_payload_t *))
{
    bst_t * bst = calloc(1, sizeof(* bst));
    bst->compare = compare;
    bst->free_payload = free_payload;
    return bst;
}

/*!
 * @brief Public interface for destroying the tree. A private function is called in its
 * stead
 * @param bst[in] bst_t
 * @param free_payload[in] Status must include either BST_FREE_PAYLOAD_TRUE or FALSE
 */
void bst_destroy(bst_t * bst, bst_status_t free_payload)
{
    traversal_free(bst, bst->root, free_payload);
    free(bst);
    bst = NULL;
}


bst_status_t bst_remove(bst_t * bst, node_payload_t * payload)
{
    node_payload_t * node_payload = bst_get_node(bst, payload);
    if (NULL == node_payload)
    {
        return BST_NODE_NOT_FOUND;
    }

    bst->root = remove_node(bst->root, payload, bst);
    bst->free_payload(node_payload);
    return BST_INSERT_SUCCESS;
}

/*!
 * @brief Recursive function to find the node to remove. There is 3 special cases that
 * the function uses to know how to promote a node.
 * @param node
 * @param payload
 * @param bst
 * @return
 */
static node_t * remove_node(node_t * node, node_payload_t * payload, bst_t * bst)
{
    if (NULL == node)
    {
        return NULL;
    }

    bst_compare_t result = bst->compare(node->key, payload);

    if (BST_LT == result)
    {
        node->left_child = remove_node(node->left_child, payload, bst);
    }
    else if (BST_GT == result)
    {
        node->right_child = remove_node(node->right_child, payload, bst);
    }
    else if (BST_EQ == result)
    {
        /*
         * The following two IF statements check if the left or right node are NULL. If
         * they are, return the opposite. Returning will assign that node to one of the
         * two if statements above i.e. "node->right_child = remove();"
         */
        if ((NULL == node->left_child) || (NULL == node->right_child))
        {
            // free the payload before returning the right or left child
//            bst->free_payload(node->key);

            if (NULL == node->right_child)
            {
                node_t * child_node = node->left_child;
                free(node);
                return child_node;
            }
            else
            {
                node_t * child_node = node->right_child;
                free(node);
                return child_node;

            }
        }
        else
        {
            /*
             * This section will return the NEW child node for the functions ABOVE. If
             * either child node of the current node is NULL, it will return the opposite
             * making that node the new child of one of the parent nodes above.
             *
             * To correctly promote a node, you either want the largest node value
             * on the current nodes left subtree or the smallest node to the right
             * subtree. The approach will instead check the height to make sure the tree
             * is balanced.
             * https://github.com/williamfiset/Algorithms/blob/a6e72cd6c1c7e5a4fde916c4140d6d2b4076762e/src/main/java/com/williamfiset/algorithms/datastructures/balancedtree/AVLTreeRecursive.java#L291
             */
            if (node->left_child->height > node->right_child->height)
            {
                // Find the MAX RIGHT value starting from the current nodes LEFT
                node_t * promote_node = find_max_payload(node->left_child);

                // free the target nodes payload
//                bst->free_payload(node->key);

                // set new payload to node
                node->key = promote_node->key;

                // Seek out the new key we set to delete the old one
                node->left_child = remove_node(node->left_child, node->key, bst);

                // free promoted node since we moved payload to current node
//                free(promote_node);
            }
            else
            {
                // Find the MAX LEFT value starting from the current nodes RIGHT
                node_t * promote_node = find_min_payload(node->right_child);

                // free the target nodes payload
//                bst->free_payload(node->key);

                // set new payload to node
                node->key = promote_node->key;

                // Seek out the new key we set to delete the old one
                node->right_child = remove_node(node->right_child, node->key, bst);

                // free promoted node since we moved payload to current node
//                free(promote_node);
            }

        }
    }
    set_height(node);
    node = balance_tree(node, bst);
    return node;

}






/*!
 * @brief Create a new node with the given node_payload. Iterate over the tree using the
 * compare function supplied to the BST object and push the new node onto the tree.
 * @param bst[in] bst_t tree pointer
 * @param payload[in] node_payload_t payload used for the new node
 * @param replace[in] Option to replace or ignore equivalent nodes
 * @return bst_status_t of the operation
 */
bst_status_t bst_insert(bst_t * bst, node_payload_t * payload, bst_status_t replace)
{
    bst->root = insert_node(bst->root, payload, replace, bst);
    return BST_ROTATION_SUCCESS;
}

/*!
 * @brief Public function to find a given payload by utilizing the registered compare
 * callback function. For this reason, primitives can not be used as a search term. A
 * whole node_payload_t has to be created for the purpose of finding the right node.
 * @param bst[in] bst_t
 * @param payload[in] node_payload_t this is a "temp" payload to utilize the key for the
 * compare function
 * @return Returns the pointer to the note_payload_t if found, otherwise a NULL is returned
 */
node_payload_t * bst_get_node(bst_t * bst, node_payload_t * payload)
{
    node_t * node = search_node(bst, bst->root, payload);
    if (NULL == node)
    {
        return NULL;
    }
    return node->key;
}

/*!
 * @brief This public function serves as a "test" for proper rotation. Rotations are normally
 * executed automatically by the tree inserts
 * @param bst[in] bst_t
 * @param payload[in] node_payload_t
 * @param side[in] BST_ROTATE_LEFT BST_ROTATE_RIGHT
 * @return bst_status_t indicating if rotation was a success
 */
bst_status_t rotate(bst_t * bst, node_payload_t * payload, bst_status_t side)
{
    node_t * node = search_node(bst, bst->root, payload);

    if (NULL == node)
    {
        return BST_SEARCH_FAILURE;
    }

    if (BST_ROTATE_RIGHT == side)
    {
        right_rotation(node, bst);
    }
    else if (BST_ROTATE_LEFT == side)
    {
        left_rotation(node, bst);
    }
    else
    {
        return BST_ROTATE_FAILURE;
    }

    return BST_ROTATION_SUCCESS;

}


/*!
 * @brief Function that calls the internal traversal function and calls the callback
 * function passed to each node to be processed.
 * @param bst[in] bst_t
 * @param type[in] Type of traversal
 * @param callback[in] Pointer to external supplied function for node processing
 */
void bst_traversal(bst_t * bst, bst_traversal_t type, void (* callback)(node_payload_t *, void *), void * void_ptr)
{
    switch (type)
    {
        case BST_IN_ORDER:
            in_order_traversal_node(bst->root, callback, void_ptr);
            break;
        case BST_POST_ORDER:
            post_order_traversal_node(bst->root, callback, void_ptr);
            break;
        case BST_PRE_ORDER:
            pre_order_traversal_node(bst->root, callback, void_ptr);
            break;
        default:
            break;
    }
}

/*!
 * @brief Recursion function to travel through the tree
 * @param node[in] The current node being inspected
 * @param function[in] External function to call on each node
 */
static void in_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr)
{
    if (NULL == node)
    {
        return;
    }
    in_order_traversal_node(node->left_child, function, void_ptr);
    function(node->key, void_ptr);
    in_order_traversal_node(node->right_child, function, void_ptr);
}

/*!
 * @brief Recursion function to travel through the tree
 * @param node[in] The current node being inspected
 * @param function[in] External function to call on each node
 */
static void pre_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr)
{
    if (NULL == node)
    {
        return;
    }
    function(node->key, void_ptr);
    pre_order_traversal_node(node->left_child, function, void_ptr);
    pre_order_traversal_node(node->right_child, function, void_ptr);
}

static void post_order_traversal_node(node_t * node, void (* function)(node_payload_t *, void *), void * void_ptr)
{
    if (NULL == node)
    {
        return;
    }
    post_order_traversal_node(node->left_child, function, void_ptr);
    post_order_traversal_node(node->right_child, function, void_ptr);
    function(node->key, void_ptr);
}

/*!
 * @brief Recursion function to free each node
 * @param bst[in] bst_t
 * @param node[in] node_t
 * @param free_payload[in] bst_status_t should include either BST_FREE_PAYLOAD_FALSE or TRUE
 */
static void traversal_free(bst_t * bst, node_t * node, bst_status_t free_payload)
{
    if (NULL == node)
    {
        return;
    }
    traversal_free(bst, node->left_child, free_payload);
    traversal_free(bst, node->right_child, free_payload);
    free_node(bst, node, free_payload);
}

/*!
 * @brief Function frees the node_payload_t by calling the bst->free_payload then it frees
 * the node itself
 * @param bst[in] bst_t
 * @param node[in] node_t
 */
static void free_node(bst_t * bst, node_t * node, bst_status_t free_payload)
{

    if (BST_FREE_PAYLOAD_TRUE == free_payload)
    {
        bst->free_payload(node->key);
        free(node);
    }
    else if (BST_FREE_PAYLOAD_FALSE == free_payload)
    {
        free(node);
    }
    else
    {
        fprintf(stderr, "Invalid bst_status_t option chosen. Please read the documentation.");
    }
}
/*!
 * @brief Function to recursively printout the nodes in a manner that you can visually see
 * in the terminal
 * @param node[in] node_t
 * @param space[in] Value of how many spaces to print
 * @param callback[in] A callback function to print out the data
 */
static void print_2d_iter(node_t * node, int space, void(*callback)(node_payload_t*))
{
    if (NULL == node)
    {
        return;
    }

    space += 10;
    print_2d_iter(node->right_child, space, callback);

    printf("\n");
    for (int i = 10; i < space; i++)
    {
        printf(" ");
    }
    callback(node->key);

    print_2d_iter(node->left_child, space, callback);
}

static node_t * right_rotation(node_t * node, bst_t * bst)
{
    node_t * new_root = node->left_child;
    node->left_child = new_root->right_child;
    new_root->right_child = node;

    set_height(node);
    set_height(new_root);
    return new_root;
}

static node_t * left_rotation(node_t * node, bst_t * bst)
{
    node_t * new_root = node->right_child;
    node->right_child = new_root->left_child;
    new_root->left_child = node;

    set_height(node);
    set_height(new_root);
    return new_root;

}

/*!
 * @brief Private function used in conjunction with bst_get_node. Read description there
 * for more information.
 * @param bst[in] bst_t
 * @param node[in] node_t
 * @param target_payload[in] node_payload_t
 * @return Returns the node that has been found, otherwise it returns a NULL
 */
static node_t * search_node(bst_t * bst, node_t * node, node_payload_t * target_payload)
{
    // if the current node is NULL, then just return it
    if (NULL == node)
    {
        return NULL;
    }

    // compare the current node to the target payload
    bst_compare_t result = bst->compare(node->key, target_payload);

    // check the result, and either recurse or return
    if (BST_EQ == result)
    {
        return node;
    }
    else if (BST_LT == result)
    {
        return search_node(bst, node->left_child, target_payload);
    }
    else
    {
        return search_node(bst, node->right_child, target_payload);
    }
}
static node_t * create_node(node_payload_t * payload)
{
    // create a new_node object
    node_t * new_node = calloc(1, sizeof(* new_node));
    new_node->key = payload;
    return new_node;
}

/*!
 * @brief Function is used to check for null or return height value
 * @param node[in] node_t
 * @return return the height of a node
 */
static int get_height(node_t * node)
{
    if (NULL == node)
    {
        return -1;
    }
    else
    {
        return node->height;
    }
}

static int node_max(int left, int right)
{
    return (left > right) ? left : right;
}

static void set_height(node_t * node)
{
    node->height = node_max(
        get_height(node->left_child),
        get_height(node->right_child)
    ) + 1;
}

static node_t * insert_node(node_t * node, node_payload_t * payload, bst_status_t replace, bst_t * bst)
{
    // if the current node is NULL, create a node for it
    if (NULL == node)
    {
        return create_node(payload);
    }

    // run the comparison function supplied
    bst_compare_t result = bst->compare(node->key, payload);
    if (BST_LT == result)
    {
        node->left_child = insert_node(node->left_child, payload, replace, bst);
    }
    else if (BST_GT == result)
    {
        node->right_child = insert_node(node->right_child, payload, replace, bst);
    }
    // This
    else if (BST_EQ == result)
    {
        if (BST_REPLACE_TRUE == replace)
        {
            free(node->key);
            node->key = payload;
        }
    }

    set_height(node);
    node = balance_tree(node, bst);

    return node;
}

/*!
 * @brief Iterate through the right side of given node until NULL is reached then return
 * the key of that node
 * @param node[in] node_t
 * @return Returns the node_payload_t of the last node on the right side of a node given
 */
static node_t * find_max_payload(node_t * node)
{
    while (NULL != node->right_child)
    {
        node = node->right_child;
    }
    return node;
}

/*!
 * @brief Iterate through the right side of given node until NULL is reached then return
 * the key of that node
 * @param node[in] node_t
 * @return Returns the node_payload_t of the last node on the right side of a node given
 */
static node_t * find_min_payload(node_t * node)
{
    while (NULL != node->left_child)
    {
        node = node->left_child;
    }
    return node;
}

static node_t * balance_tree(node_t * node, bst_t * bst)
{
    if (is_left_heavy(node))
    {
        // check if a left rotation is required before right
        if (get_balance_factor(node->left_child) < 0)
        {
            node->left_child = left_rotation(node->left_child, bst);
        }
        // always perform a right rotation
        return right_rotation(node, bst);
    }

    else if (is_right_heavy(node))
    {
        // check if right rotation is needed before left
        if (get_balance_factor(node->right_child) > 0)
        {
            node->right_child = right_rotation(node->right_child, bst);
        }
        // always perform a left rotation
        return left_rotation(node, bst);
    }

    // if execution gets here, then the tree is already balanced
    return node;
}

static bool is_right_heavy(node_t * node)
{
    return (get_balance_factor(node) < -1);
}
static bool is_left_heavy(node_t * node)
{
    return (get_balance_factor(node) > 1);
}

/*!
 * @brief Private function to get the height of the current node while checking for nulls
 * @param node[in] node_t
 * @return Node balance factor value
 */
static int get_balance_factor(node_t * node)
{
    return (NULL == node) ? 0 : get_height(node->left_child) - get_height(node->right_child);
}
