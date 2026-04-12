#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <GLFW/glfw3.h>

#include "maze_generator.h"


typedef void (*step)(maze_t *maze);


GLFWwindow *init_graphics();
void free_graphics(GLFWwindow *window);
cell_t *get_collection();
void draw_loop(GLFWwindow *window);
// void draw(maze_t *maze);
// void update(maze_t *maze, step fn);

#endif

