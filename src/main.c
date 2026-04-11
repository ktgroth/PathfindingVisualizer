
#include <stdio.h>

#include "include/graphics.h"
#include "include/maze_generator.h"
#include "include/breadth_first.h"
#include "include/depth_first.h"


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
    maze_t maze;
    size_t n = 25;

    size_t N = 2*n + 1;
    if (init_maze(&maze, N))
        return 1;

    maze_t bf;
    maze_t df;
    copy_maze(&maze, &bf);
    copy_maze(&maze, &df);

    if (!init_graphics(N))
        return 1;

    bf_search(&bf);
    df_search(&df);
    free_graphics();

    print_maze(&maze);
    printf("\n\n");
    print_maze(&bf);
    printf("\n\n");
    print_maze(&df);
    printf("\n\n");

    free_maze(&maze);
    free_maze(&bf);
    free_maze(&df);
    return 0;
}

