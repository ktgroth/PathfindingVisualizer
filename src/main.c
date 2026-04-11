
#include <stdio.h>

#include "include/maze_generator.h"


int main(int argc, char **argv)
{
    maze_t maze;
    size_t n = 25;

    size_t N = 2*n + 1;
    if (init_maze(&maze, N))
        return 1;


    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
            printf("%c", maze.walls[IX(i, j, N)] == WALL ? '#' : ' ');
        printf("\n");
    }

    return 0;
}

