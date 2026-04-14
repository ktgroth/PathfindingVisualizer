#ifndef HEURISTIC_H
#define HEURISTIC_H


#include "maze_generator.h"


typedef float (*heuristic)(cell_t pos, cell_t end);


float manhattan_dist(cell_t pos, cell_t end);
float euclidean_dist(cell_t pos, cell_t end);
float chebyshev_dist(cell_t pos, cell_t end);

#endif

