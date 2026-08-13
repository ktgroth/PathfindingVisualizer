#ifndef HEURISTIC_H
#define HEURISTIC_H


#include "maze_generator.h"


#define SQRT2 1.41421356f

typedef float (*heuristic)(cell_t pos, cell_t end);


float manhattan_dist(cell_t pos, cell_t end);
float euclidean_dist(cell_t pos, cell_t end);
float squared_euclidean_dist(cell_t pos, cell_t end);
float chebyshev_dist(cell_t pos, cell_t end);
float cosine_dist(cell_t pos, cell_t end);
float octile_dist(cell_t pos, cell_t end);
float minkowski_dist(cell_t pos, cell_t end);

#endif

