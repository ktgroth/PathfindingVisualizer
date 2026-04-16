
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/breadth_first.h"


static cell_t *queue;
extern int size;
static int queue_start, queue_end;
static int *parent;


void init_bf(maze_t *maze, int x, int y)
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

    queue = get_collection();
    queue[0] = (cell_t){ x, y };
    queue_start = 0, queue_end = 1;

    maze->walls[IX(x, y, maze->N)] |= VISITED;
}

void bf_step(maze_t *maze)
{
    if (queue_start >= queue_end)
        return;

    int N = maze->N;
    cell_t curr = queue[queue_start++];
    int x = curr.x, y = curr.y;
    int cidx = IX(x, y, N);
    maze->walls[cidx] |= VISITED;

    clear_flag_everywhere(maze, PATH_CURRENT);

    if (maze->walls[cidx] & END)
    {
        queue_start = queue_end + 1;

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
        if (maze->walls[idx] & (ADDED | VISITED | WALL))
            continue;

        parent[idx] = cidx;
        maze->walls[idx] |= ADDED;
        if (queue_end >= size)
        {
            cell_t *new_queue = realloc(queue, size * 2);
            if (!new_queue)
            {
                perror("Changing queue size");
                return;
            }

            size *= 2;
            queue = new_queue;
            set_collection(queue);
        }

        queue[queue_end++] = (cell_t){ nx, ny };
    }
}

