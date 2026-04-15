
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/depth_first.h"


static cell_t *stack;
static int stack_top = 0;
static int *parent;

void init_df(maze_t *maze, int x, int y)
{
    int N = maze->N;
    parent = (int *)malloc(N*N * sizeof(int));
    if (!parent)
    {
        perror("Allocating parent array");
        return;
    }
    for (int i = 0; i < N*N; ++i)
        parent[i] = -1;

    stack = get_collection();
    stack[0] = (cell_t){ x, y };
    stack_top = 1;

    maze->walls[IX(x, y, maze->N)] |= VISITED;
}

void df_step(maze_t *maze)
{
    if (stack_top < 0)
        return;

    int N = maze->N;
    cell_t curr = stack[--stack_top];
    int x = curr.x, y = curr.y;
    int cidx = IX(x, y, N);

    clear_flag_everywhere(maze, PATH_CURRENT);

    if (maze->walls[cidx] & END)
    {
        stack_top = -1;

        int pidx = parent[cidx];
        maze->walls[cidx] |= PATH_FINAL;
        while (pidx > -1)
        {
            maze->walls[pidx] |= PATH_FINAL;
            pidx = parent[pidx];
        }

        return;
    }

    int pidx = parent[cidx];
    maze->walls[cidx] |= PATH_CURRENT;
    while (pidx > -1)
    {
        maze->walls[pidx] |= PATH_CURRENT;
        pidx = parent[pidx];
    }

    for (int dir = 0; dir < dir_size; ++dir)
    {
        int dx = ddirs[dir][0], dy = ddirs[dir][1];
        int nx = x + dx, ny = y + dy;

        if (nx < 0 || ny < 0 || nx >= N || ny >= N)
            continue;

        int idx = IX(nx, ny, N);
        if (maze->walls[idx] & (VISITED | WALL))
            continue;

        maze->walls[idx] |= VISITED;
        parent[idx] = cidx;
        stack[stack_top++] = (cell_t){ nx, ny };
    }
}

