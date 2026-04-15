
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "include/astar.h"
#include "include/heap.h"
#include "include/heuristic.h"


#define SQRT2 1.41421356f

static cell_t start, end;
static heap_t heap;
static float *gscore;
static int *parent;

extern volatile heuristic h;

void init_astar(maze_t *maze, int sx, int sy, int ex, int ey)
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

    if (gscore)
    {
        free_heap(&heap);
        free(gscore);
        gscore = NULL;
    }

    if (init_heap(&heap, N))
        return;

    gscore = (float *)calloc(N*N, sizeof(float));
    if (!gscore)
    {
        free_heap(&heap);
        return;
    }

    for (int i = 0; i < N*N; ++i)
        gscore[i] = INFINITY;

    start = (cell_t){ sx, sy };
    end = (cell_t){ ex, ey };

    gscore[IX(sx, sy, N)] = 0;
    float hscore = h(start, end);
    heap_insert(&heap, start, hscore, hscore);
}

void astar_step(maze_t *maze)
{
    if (heap.size <= 0)
        return;

    int N = maze->N;
    cell_t curr = heap_extract_min(&heap);
    int x = curr.x, y = curr.y;
    int cidx = IX(x, y, N);

    if (maze->walls[cidx] & VISITED)
        return;

    clear_flag_everywhere(maze, PATH_CURRENT);

    if (maze->walls[cidx] & END)
    {
        heap.size = 0;

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

    maze->walls[cidx] |= VISITED;
    for (int dir = 0; dir < dir_size; ++dir)
    {
        int dx = ddirs[dir][0], dy = ddirs[dir][1];
        int nx = x + dx, ny = y + dy;

        if (nx < 0 || ny < 0 || nx >= N || ny >= N)
            continue;

        int idx = IX(nx, ny, N);
        if (maze->walls[idx] & (VISITED | WALL))
            continue;

        float step_cost = (dx != 0 && dy != 0) ? SQRT2 : 1.0f;
        float tentative_g = gscore[cidx] + step_cost;
        if (tentative_g < gscore[idx])
        {
            cell_t next = (cell_t){ nx, ny };
            gscore[idx] = tentative_g;
            float hscore = h(next, end);
            parent[idx] = cidx;

            if (maze->walls[idx] & END)
                heap_insert(&heap, next, 0, 0);
            else
                heap_insert(&heap, next, gscore[idx] + hscore, hscore);
        }
    }
}

