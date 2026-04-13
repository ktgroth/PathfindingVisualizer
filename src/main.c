
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "include/graphics.h"
#include "include/maze_generator.h"
#include "include/astar.h"

void print_maze(maze_t *maze)
{
    int N = maze->N;
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            int8_t info = maze->walls[IX(j, i, N)];
            if (info & WALL)
                printf("#");
            else if (info & (START | END))
                printf("@");
            else if (info & VISITED)
                printf("*");
            else
                printf(" ");
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    GLFWwindow *window = init_graphics();
    if (!window)
        return 1;
    draw_loop(window);

    return 0;
}

