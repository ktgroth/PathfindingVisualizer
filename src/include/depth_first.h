#ifndef DEPTH_FIRST_H
#define DEPTH_FIRST_H

#include "graphics.h"
#include "maze_generator.h"


void init_df(maze_t *maze, int x, int y);
void df_step(maze_t *maze);
void df_clean();

#endif

