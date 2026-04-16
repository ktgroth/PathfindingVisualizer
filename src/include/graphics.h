#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <GLFW/glfw3.h>

#include "maze_generator.h"


typedef void (*step)(maze_t *maze);


GLFWwindow *init_graphics();
void free_graphics(GLFWwindow *window);
cell_t *get_collection();
void set_collection(cell_t *coll);
void draw_loop(GLFWwindow *window);

#endif

