
#include <stdio.h>

#include "include/maze_generator.h"
#include "include/breadth_first.h"
#include "include/depth_first.h"


int main(int argc, char **argv)
{
    maze_t maze;
    size_t n = 25;

    size_t N = 2*n + 1;
    if (init_maze(&maze, N))
        return 1;

    df_search(&maze);
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            switch (maze.walls[IX(j, i, N)])
            {
                case WALL:
                    printf("#");
                    break;

                case OPEN:
                    printf(" ");
                    break;

                case VISITED:
                    printf("*");
                    break;
            }
        }
        printf("\n");
    }

    return 0;
}

