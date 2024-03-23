#include "frameprocess.h"

#include "frame.h"


void print_neighbors(const int* neighbors, int node, int max_neighbors)
{
    printf("Neighbor List for N%i: ", node);

    for (int p = 0; p < max_neighbors; ++p)
    {
        int neighbor = neighbors[p + node * max_neighbors];
        if (neighbor != -1)
        {
            printf("%i ", neighbor);
        }
    }

    printf("\n");
}

void print_neighbor_colors(const int* neighbors, const struct Node* nodes, int node, int max_neighbors)
{
    printf("Neighbor Colors for N%i: ", node);

    for (int p = 0; p < max_neighbors; ++p)
    {
        int neighbor = neighbors[p + node * max_neighbors];
        if (neighbor != -1)
        {
            printf("%i ", nodes[neighbor].multicolor);
        }
    }

    printf("\n");
}


void frame_preprocess(struct Frame* frame)
{
    //Typical Red-Black method does not work for FEM meshes without some preprocessing

    // The goal is to assign each node a "color" such that no element has the
    // same color at both nodes after doing this all the nodes with the same 
    // color are decoupled from nodes of other colors and can be processed 
    // separately by multiple processes/threads after sorting the rows by color

    // At the moment the graph is stored as a list of edges each having a start
    // and end point. this is not an easy representation use since for a given
    // node we do not have an easy way to check its neighbors so the first step
    // is to build a data structure that allows checking neighbors

    // Simple Adjancency list with fixed size
    const int max_neighbors = 8;
    int degree = 0;

    int neighbors_count = frame->node_count * max_neighbors;
    int* neighbors = malloc(sizeof(*neighbors) * neighbors_count);

    // Initialize to -1 to signify no neighbors
    for (int i = 0; i < neighbors_count; ++i)
    {
        neighbors[i] = -1;
    }

    for (int i = 0; i < frame->node_count; ++i)
    {
        frame->nodes[i].multicolor = 0;
    }

    for (int i = 0; i < frame->element_count; ++i)
    {
        int n1 = frame->elements[i].node1;
        int n2 = frame->elements[i].node2;

        // Check the list of n1's neighbors to see if n2 needs to be added
        for (int n = 0; n < max_neighbors; ++n)
        {
            // Update the graph degree if a node has a higher degree
            if (n > degree)
            {
                degree = n;
            }

            if (neighbors[n + n1 * max_neighbors] == n2)
            {
                // List already contains n2
                break;
            }
            else if (neighbors[n + n1 * max_neighbors] == -1)
            {
                // Empty space found without having found n2 yet
                // so add n2 to the list of n1's neighbors
                neighbors[n + n1 * max_neighbors] = n2;
                break;
            }
            else if (n == max_neighbors - 1)
            {
                // End of list reached without finding space to place n2
                fprintf(stderr, "Error building Adjacency List: max_neighbors exceeded\n");
                print_neighbors(neighbors, n1, max_neighbors);
                //return;
            }
        }

        // Repeat the same process for n2
        for (int n = 0; n < max_neighbors; ++n)
        {
            // Update the graph degree if a node has a higher degree
            if (n > degree)
            {
                degree = n;
            }

            if (neighbors[n + n2 * max_neighbors] == n1)
            {
                // List already contains n2
                break;
            }
            else if (neighbors[n + n2 * max_neighbors] == -1)
            {
                // Empty space found without having found n2 yet
                // so add n2 to the list of n1's neighbors
                neighbors[n + n2 * max_neighbors] = n1;
                break;
            }
            else if (n == max_neighbors - 1)
            {
                // End of list reached without finding space to place n1
                fprintf(stderr, "Error building Adjacency List: max_neighbors exceeded\n");
                print_neighbors(neighbors, n2, max_neighbors);
                //return;
            }
        }
    }

    printf("Graph Degree: %i\n", degree);

    // Want to have at least degree + 1 colors available to ensure no node has
    // a neighbor with the same color
    int num_colors = degree + 1;

    // Assign a multicolor value to each node such that no node has a color in common
    // with any of it's neighbors
    for (int i = 0; i < frame->node_count; ++i)
    {
        // Look through the set of colors and pick the first color that is not
        // a color of any of the neighbors. This yields an uneven distribution
        // that can be fixed later by applying randomness
        for (int color = 1; color <= num_colors; ++color)
        {
            int available = 1;
            for (int n = 0; n < degree; ++n)
            {
                // Check if a color is already used by a neighbor
                int node = neighbors[n + i * max_neighbors];
                if (node != -1 && color == frame->nodes[node].multicolor)
                {
                    available = 0;
                    break;
                }
            }

            // Set the nodes color if available
            if (available)
            {
                frame->nodes[i].multicolor = color;
                break;
            }
        }

        if (frame->nodes[i].multicolor == 0)
        {
            fprintf(stderr, "Error assigning multicolor: Failed to find available multicolor\n");
            print_neighbor_colors(neighbors, frame->nodes, i, max_neighbors);
        }
    }
}
