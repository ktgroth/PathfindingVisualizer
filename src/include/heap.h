#ifndef PRIORITY_HEAP_H
#define PRIORITY_HEAP_H


#include "maze_generator.h"


typedef struct
{
    cell_t pos;
    float f;
    float h;
} node_t;

typedef struct
{
    node_t *queue;
    int size;
} heap_t;


int init_heap(heap_t *heap, int N);
void free_heap(heap_t *heap);
void heap_insert(heap_t *heap, cell_t pos, float f, float h);
cell_t heap_extract_min(heap_t *heap);

#endif

