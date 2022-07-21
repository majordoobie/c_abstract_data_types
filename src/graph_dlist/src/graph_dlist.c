#include <graph_dlist.h>
#include <stdlib.h>
#include <assert.h>

typedef struct graph_t
{
    dlist_t * nodes;
    graph_mode_t graph_mode;
    dlist_match_t (* compare_func)(void *, void *);
} graph_t;

typedef struct node_t
{
    void * data;
    dlist_t * edges;
} node_t;

typedef struct
{
    uint32_t weight;
    node_t * to_node;
} edge_t;

static edge_t * get_edge(node_t * to_node, uint32_t weight);

/*!
 * @brief Initialize a adjacency list graph structure that uses a double linked
 * list for both the nodes and edges. When time allows, it will be best to
 * substitute the list of nodes for a hast table of nodes for significantly
 * increase lookup times.
 *
 * @param graph_mode GRAPH_DIRECTIONAL_TRUE OR GRAPH_DIRECTIONAL_FALSE
 * @param compare_func Function pointer for making comparisons between nodes
 * @return Pointer to graph object or NULL with a stdout message
 */
graph_t * graph_init(graph_mode_t graph_mode,
                     dlist_match_t (* compare_func)(void *, void *))
{
    graph_t * graph = (graph_t *)malloc(sizeof(graph_t));
    if (INVALID_PTR == verify_alloc(graph))
    {
        return NULL;
    }

    dlist_t * dlist = dlist_init(compare_func);
    if (NULL == dlist)
    {
        graph_destroy(graph);
        return NULL;
    }

    * graph = (graph_t){
        .nodes          = dlist,
        .compare_func   = compare_func,
        .graph_mode     = graph_mode
    };

    return graph;
}

/*!
 * @brief Add an edge between the two given nodes if they exist. A weight value
 * is used to indicate the weight between the two nodes. This option can be
 * omited with the NO_WEIGHT enum.
 *
 * @param graph Pointer to graph object
 * @param source_node Pointer to the node_t object
 * @param target_node Pointer to the node_t object
 * @param weight Value of the edge weight. Use NO_WEIGHT for default weight of 0
 * @return
 * GRAPH_SUCCESS  If operation was successful. GRAPH_EDGE_ALREADY_EXISTS if
 * the edge already exists.
 */
graph_opt_t graph_add_edge(graph_t * graph,
                           node_t * source_node,
                           node_t * target_node,
                           uint32_t weight)
{
    assert(graph);
    assert(source_node);
    assert(target_node);

    // First step is to see if the source node already has the edge for that
    // target node
    if (NULL == dlist_get_by_value(source_node->edges, target_node))
    {
        edge_t * edge = get_edge(target_node, weight);
        dlist_append(source_node->edges, edge);
    }
    else
    {
        return GRAPH_EDGE_ALREADY_EXISTS;
    }

    // finally, if the graph mode is unidirectional, then add the edge
    // going the opposite direction
    if (GRAPH_DIRECTIONAL_FALSE == graph->graph_mode)
    {
        // only add it if the edge does not exist
        if (NULL == dlist_get_by_value(target_node->edges, source_node))
        {
            edge_t * edge = get_edge(source_node, weight);
            dlist_append(target_node->edges, edge);
        }
    }

    return GRAPH_SUCCESS;
}

/*!
 * @brief Destroy the graph object with the option to free the data nodes in the
 * graph using function pointer that handles freeing the data nodes.
 *
 * @param graph Pointer to the graph object
 */
void graph_destroy(graph_t * graph, void (*free_func)(void *))
{
    if (NULL != graph->nodes)
    {
        if (NULL == free_func)
        {
            dlist_destroy(graph->nodes);
        }
        else
        {
            dlist_destroy_free(graph->nodes, free_func);
        }
    }
    free(graph);
}

/*!
 * @brief Create an edge object
 * @param to_node Pointer to the node_t object that the edge leads to
 * @param weight Weight of the edge
 * @return Pointer to a edge object or NULL if invalid with a error message to
 * stderr.
 */
static edge_t * get_edge(node_t * to_node, uint32_t weight)
{
    edge_t * edge = (edge_t *)malloc(sizeof(edge_t));
    if (INVALID_PTR == verify_alloc(edge))
    {
        return NULL;
    }

    * edge = (edge_t){
        .to_node    = to_node,
        .weight     = weight
    };
    return edge;
}











