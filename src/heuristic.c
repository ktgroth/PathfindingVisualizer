
#include <stdlib.h>
#include <math.h>

#include "include/heuristic.h"


float manhattan_dist(cell_t pos, cell_t end)
{
    return abs(end.x - pos.x) + abs(end.y - pos.y);
}

float euclidean_dist(cell_t pos, cell_t end)
{
    int dx = end.x - pos.x;
    int dy = end.y - pos.y;
    return sqrt(dx*dx + dy*dy);
}

float chebyshev_dist(cell_t pos, cell_t end)
{
    int dx = abs(end.x - pos.x);
    int dy = abs(end.y - pos.y);
    return (float)(dx > dy ? dx : dy);
}

