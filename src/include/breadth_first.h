#ifndef BREADTH_FIRST_H
#define BREADTH_FIRST_H

#include "graphics.h"
#include "maze_generator.h"


void init_bf(maze_t *maze, int x, int y);
void bf_step(maze_t *maze);
void bf_clean();

#endif

