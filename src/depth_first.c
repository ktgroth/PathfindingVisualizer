
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/depth_first.h"


static cell_t *stack;
static int stack_top = 0;

void init_df(maze_t *maze, int x, int y)
{
    stack = get_collection();
    stack[0] = (cell_t){ x, y };
    stack_top = 1;

    maze->walls[IX(x, y, maze->N)] |= VISITED;
}

void df_search(maze_t *maze)
{
    int N = maze->N;
    maze->walls[IX(1, 0, N)] |= VISITED;

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

            int idx = IX(nx, ny, N);
            if (maze->walls[idx] & (VISITED | WALL))
                continue;

            maze->walls[idx] |= VISITED;
            if (maze->walls[idx] & END)
                return;

            stack[stack_top++] = (cell_t){ nx, ny };
        }
    }
}

void df_step(maze_t *maze)
{
    if (stack_top < 0)
        return;

    int N = maze->N;
    cell_t curr = stack[--stack_top];
    int x = curr.x, y = curr.y;

    for (int dir = 0; dir < 4; ++dir)
    {
        int dx = ddirs[dir][0], dy = ddirs[dir][1];
        int nx = x + dx, ny = y + dy;

        if (nx < 0 || ny < 0 || nx >= N || ny >= N)
            continue;

        int idx = IX(nx, ny, N);
        if (maze->walls[idx] & (VISITED | WALL))
            continue;

        maze->walls[idx] |= VISITED;
        if (maze->walls[idx] & END)
        {
            stack_top = -1;
            return;
        }

        stack[stack_top++] = (cell_t){ nx, ny };
    }
}

