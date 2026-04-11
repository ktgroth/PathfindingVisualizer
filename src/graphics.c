
#include <stdio.h>
#include <stdlib.h>

#include "include/graphics.h"


static cell_t *collection;


int init_graphics(int N)
{
    collection = (cell_t *)malloc(N*N * sizeof(cell_t *));
    if (!collection)
    {
        perror("Allocating collection");
        return 0;
    }

    return 1;
}

void free_graphics()
{
    if (!collection)
        return;

    free(collection);
}

cell_t *get_collection()
{
    return collection;
}

void draw()
{

}

void update()
{

}

