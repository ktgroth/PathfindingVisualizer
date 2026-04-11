#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "maze_generator.h"


static const float quad[] = {
    -1.0f, -1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,

    -1.0f, -1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  0.0f,  1.0f
};


static volatile int fb_width = 1080, fb_height = 720;
static volatile int win_width = 1080, win_height = 720;
static volatile double mx = 0, my = 0;


int init_graphics(int N);
void free_graphics();
cell_t *get_collection();
void draw();
void update();

#endif

