
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/depth_first.h"


static cell_t *stack;
extern int size, done;
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
    if (!stack)
    {
        stack = (cell_t *)malloc(N*N * sizeof(cell_t));
        if (!stack)
        {
            perror("Allocating new Collection");
            return;
        }

        size = N*N;
        set_collection(stack);
    }

    stack[0] = (cell_t){ x, y };
    stack_top = 1;

    maze->walls[IX(x, y, maze->N)] |= VISITED;
    done = 0;
}

void df_step(maze_t *maze)
{
    if (done || stack_top <= 0)
        return;

    int N = maze->N;
    cell_t curr = stack[--stack_top];
    int x = curr.x, y = curr.y;
    int cidx = IX(x, y, N);
    maze->walls[cidx] |= VISITED;

    clear_flag_everywhere(maze, PATH_CURRENT);

    if (maze->walls[cidx] & END)
    {
        int pidx = parent[cidx];
        maze->walls[cidx] |= PATH_FINAL;
        while (pidx > -1)
        {
            maze->walls[pidx] |= PATH_FINAL;
            pidx = parent[pidx];
        }

        done = 1;
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
        if (maze->walls[idx] & (ADDED | VISITED | WALL))
            continue;

        parent[idx] = cidx;
        if (stack_top >= size)
        {
            int new_size = size * 2;
            printf("OLD SIZE: %d\n", size);

            cell_t *new_stack = realloc(stack, new_size * sizeof(cell_t));
            if (!new_stack)
            {
                perror("Changing stack size");
                return;
            }

            size = new_size;
            stack = new_stack;
            set_collection(stack);
            printf("NEW SIZE: %d\n", size);
        }

        maze->walls[idx] |= ADDED;
        stack[stack_top++] = (cell_t){ nx, ny };

        if (maze->walls[idx] & END)
            return;
    }
}

void df_clean()
{
    if (stack)
    {
        free(stack);
        stack = NULL;
        set_collection(NULL);
    }

    if (parent)
    {
        free(parent);
        parent = NULL;
    }
}

