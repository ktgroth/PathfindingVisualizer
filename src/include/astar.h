#ifndef ASTAR_H
#define ASTAR_H


#include "maze_generator.h"


void init_astar(maze_t *maze, int sx, int sy, int ex, int ey);
void astar_step(maze_t *maze);
void astar_clean();

#endif

