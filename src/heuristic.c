
#include <stdlib.h>

#include "include/heuristic.h"


int manhattan_dist(cell_t pos, cell_t end)
{
    return abs(end.x - pos.x) + abs(end.y - pos.y);
}

