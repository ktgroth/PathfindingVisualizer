#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "maze_generator.h"


#define BF  1
#define DF  2

extern volatile int fb_width, fb_height;
extern volatile int win_width, win_height;
extern volatile double mx, my;


typedef void (*step)(maze_t *maze);


int init_graphics(int N);
void free_graphics();
cell_t *get_collection();
void draw(maze_t *maze);
void update(maze_t *maze, step fn);

#endif

