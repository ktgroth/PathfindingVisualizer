#ifndef HEURISTIC_H
#define HEURISTIC_H


#include "maze_generator.h"


typedef int (*heuristic)(cell_t pos, cell_t end);


int manhattan_dist(cell_t pos, cell_t end);

#endif

