
#include <stdio.h>
#include <stdlib.h>

#include "include/heap.h"


int init_heap(heap_t *heap, int N)
{
    heap->queue = (node_t *)malloc(N*N * sizeof(node_t));
    if (!heap->queue)
    {
        perror("Allocating heap");
        return 1;
    }

    heap->size = 0;
    heap->capacity = N*N;
    return 0;
}

void free_heap(heap_t *heap)
{
    if (!heap || heap->queue == NULL)
        return;

    free(heap->queue);
    heap->queue = NULL;
    heap->size = 0;
}

static int node_less(node_t a, node_t b)
{
    if (a.f < b.f)
        return 1;
    if (a.f > b.f)
        return 0;

    return a.h < b.h;
}

void heap_insert(heap_t *heap, cell_t pos, float f, float h)
{
    node_t node = (node_t){ pos, f, h };

    if (heap->size >= heap->capacity)
    {
        node_t *new_queue = (node_t *)realloc(heap->queue, heap->capacity * 2);
        if (!new_queue)
        {
            perror("Changing heap size");
            return;
        }

        heap->capacity *= 2;
        heap->queue = new_queue;
    }

    int idx = heap->size++;
    heap->queue[idx] = node;

    while (idx > 0)
    {
        int pidx = (idx - 1) / 2;
        if (!node_less(heap->queue[idx], heap->queue[pidx]))
            break;

        node_t tmp = heap->queue[pidx];
        heap->queue[pidx] = heap->queue[idx];
        heap->queue[idx] = tmp;

        idx = pidx;
    }
}

cell_t heap_extract_min(heap_t *heap)
{
    int size = heap->size;
    if (size <= 0)
        return (cell_t){ -1, -1 };

    cell_t pos = heap->queue[0].pos;
    int idx = --heap->size;
    heap->queue[0] = heap->queue[idx];

    idx = 0;
    while (1)
    {
        int lidx = 2*idx + 1;
        int ridx = 2*idx + 2;
        int smallest = idx;

        if (lidx < heap->size && node_less(heap->queue[lidx], heap->queue[smallest]))
            smallest = lidx;
        if (ridx < heap->size && node_less(heap->queue[ridx], heap->queue[smallest]))
            smallest = ridx;

        if (smallest == idx)
            break;

        node_t tmp = heap->queue[smallest];
        heap->queue[smallest] = heap->queue[idx];
        heap->queue[idx] = tmp;

        idx = smallest;
    }

    return pos;
}

