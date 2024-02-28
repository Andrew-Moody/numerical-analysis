#include "frame.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"


struct Node
{
    struct vec3 pos;
    struct vec3 force;
    struct vec3 moment;
    struct vec3 displacement;
    struct vec3 rotation;
};


struct Element
{
    int node1;
    int node2;
    float elastic_modulus;
    float shear_modulus;
};


void structural(void)
{
    printf("test\n");


    int node_count = 2;

    int element_count = 1;

    // it's also recommended to not cast the result of malloc as that could hide errors and is not necessary. C++ is 
    // different because void* will not implicitly convert and needs a cast but use of malloc in C++ is a special case

    // using sizeof on the dereferenced pointer keeps the type coupled to the type of the pointer so no need to
    // change the sizeof expression if the pointer type changes and will give an error if the variable name changes
    // sizeof(*ptr) is evaluated at compile time so the pointer is not actually dereferenced and is fine to be NULL

    // it is also sometimes recommended to put the sizeof expression first since this insures multiplications are
    // done with size_t. for example if width and height are int on a 64 bit system width * height * sizeof(something)
    // could overflow the temp int result of width * height but sizeof(something) * width * height will not unless it would
    // overflow size_t regardless

    struct Node* nodes = NULL;
    size_t nodes_size = sizeof(*nodes) * node_count;
    nodes = malloc(nodes_size);

    struct Element* elements = NULL;
    size_t elements_size = sizeof(*elements) * element_count;
    elements = malloc(elements_size);

    // Define node positions
    nodes[0].pos = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[1].pos = (struct vec3){ 0.f, 0.f, 0.f };

    // Define boundary conditions (node1 is fixed, node2 has a force applied)
    nodes[0].displacement = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[0].rotation = (struct vec3){ 0.f, 0.f, 0.f };

    nodes[1].force = (struct vec3){ 1.f, 0.f, 0.f };

    // Define an element between node1 and node2
    struct Element element = { 0, 1, 1.f, 1.f };

    elements[0] = element;




    free(nodes);
    free(elements);
}