
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "include/astar.h"
#include "include/heap.h"
#include "include/heuristic.h"


#define SQRT2 1.41421356f

cell_t start, end;
heap_t heap;
float *gscore;

extern volatile heuristic h;

void init_astar(maze_t *maze, int sx, int sy, int ex, int ey)
{
    if (gscore)
    {
        free_heap(&heap);
        free(gscore);
        gscore = NULL;
    }

    int N = maze->N;
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

    maze->walls[cidx] |= VISITED;
    printf("(%d, %d) ~ (%g, %g)\n", x, y, gscore[cidx], h(curr, end));

    if (maze->walls[cidx] & END)
    {
        heap.size = 0;
        return;
    }

    for (int dir = 0; dir < 8; ++dir)
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
            heap_insert(&heap, next, gscore[idx] + hscore, hscore);
        }
    }
}

