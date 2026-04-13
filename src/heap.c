
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
    return 0;
}

void free_heap(heap_t *heap)
{
    if (!heap)
        return;

    free(heap->queue);
    heap->size = 0;
}

void heap_insert(heap_t *heap, cell_t pos, int f)
{
    int size = heap->size;
    node_t node = (node_t){ pos, f };

    if (size == 0)
    {
        heap->size = 1;
        heap->queue[0] = node;
        return;
    }

    int i;
    for (i = 0; i < size; ++i)
    {
        int ef = heap->queue[i].f;

        if (ef > f)
        {
            for (int j = size; j > i; --j)
                heap->queue[j] = heap->queue[j - 1];

            heap->queue[i] = node;
            ++heap->size;
            return;
        }
    }

    heap->queue[i] = node;
    ++heap->size;
}

cell_t heap_extract_min(heap_t *heap)
{
    int size = heap->size;
    if (size <= 0)
        return (cell_t){ -1, -1 };

    cell_t pos = heap->queue[0].pos;
    --heap->size;
    for (int i = 0; i < heap->size; ++i)
        heap->queue[i] = heap->queue[i + 1];

    return pos;
}

