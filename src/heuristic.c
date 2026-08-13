
#include <stdlib.h>
#include <math.h>

#include "include/heuristic.h"


// float manhattan_dist(cell_t pos, cell_t end);
// float euclidean_dist(cell_t pos, cell_t end);
// float squared_euclidean_dist(cell_t pos, cell_t end);
// float chebyshev_dist(cell_t pos, cell_t end);
// float cosine_dist(cell_t pos, cell_t end);
// float octile_dist(cell_t pos, cell_t end);

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

float squared_euclidean_dist(cell_t pos, cell_t end)
{
    int dx = end.x - pos.x;
    int dy = end.y - pos.y;
    return dx*dx + dy*dy;
}

float chebyshev_dist(cell_t pos, cell_t end)
{
    int dx = abs(end.x - pos.x);
    int dy = abs(end.y - pos.y);
    return (float)(dx > dy ? dx : dy);
}

float cosine_dist(cell_t pos, cell_t end)
{
    float dot = pos.x * end.x + pos.y * end.y;
    float norm_p = pos.x * pos.x + pos.y * pos.y;
    float norm_e = end.x * end.x + end.y * end.y;

    if (norm_p == 0.0f || norm_e == 0.0f)
        return 1.0f;

    float similarity = dot / (sqrt(norm_p) * sqrt(norm_e));
    if (similarity > 1.0f)
        similarity = 1.0f;
    if (similarity < -1.0f)
        similarity = -1.0f;

    return 1.0f - similarity;
}

float octile_dist(cell_t pos, cell_t end)
{
    int dx = abs(pos.x - end.x);
    int dy = abs(pos.y - end.y);

    int max = (dx > dy ? dx : dy);
    int min = (dx > dy ? dy : dx);
    return (max - min) + SQRT2 * min;
}

