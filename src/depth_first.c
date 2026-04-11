
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/depth_first.h"


void df_search(maze_t *maze)
{
    size_t N = maze->N;
    cell_t *stack = (cell_t *)malloc(N*N * sizeof(cell_t));
    if (!stack)
    {
        perror("Depth First Search");
        return;
    }

    stack[0] = (cell_t){ 1, 0 };
    int stack_top = 1;
    while (stack_top >= 0)
    {
        cell_t curr = stack[--stack_top];
        int x = curr.x, y = curr.y;

        for (int dir = 0; dir < 4; ++dir)
        {
            int dx = ddirs[dir][0], dy = ddirs[dir][1];
            int nx = x + dx, ny = y + dy;

            if (nx < 0 || ny < 0 || nx >= N || ny >= N)
                continue;

            if (maze->walls[IX(nx, ny, N)] != OPEN)
                continue;

            maze->walls[IX(nx, ny, N)] = VISITED;
            if (nx == N - 1 && ny == N - 2)
            {
                free(stack);
                return;
            }

            stack[stack_top++] = (cell_t){ nx, ny };
        }
    }

    free(stack);
}

