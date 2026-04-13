
#include <stdlib.h>
#include <limits.h>

#include "include/astar.h"
#include "include/heap.h"
#include "include/heuristic.h"

cell_t start, end;
heap_t heap;
int *gscore;

extern volatile heuristic h;

void init_astar(maze_t *maze, int sx, int sy, int ex, int ey)
{
    int N = maze->N;
    if (init_heap(&heap, N))
        return;

    gscore = (int *)calloc(N*N, sizeof(int));
    if (!gscore)
    {
        free_heap(&heap);
        return;
    }

    for (int i = 0; i < N*N; ++i)
        gscore[i] = INT_MAX;

    gscore[IX(sx, sy, N)] = 0;
    start = (cell_t){ sx, sy };
    end = (cell_t){ ex, ey };
    heap_insert(&heap, start, h(start, end));
}

void astar_search(maze_t *maze)
{
    int N = maze->N;
    init_heap(&heap, N);

    gscore = (int *)calloc(N*N, sizeof(int));
    if (!gscore)
    {
        free_heap(&heap);
        return;
    }

    for (int i = 0; i < N*N; ++i)
        gscore[i] = INT_MAX;

    start = (cell_t){ 1, 0 }, end = (cell_t){ N - 1, N - 2 };
    gscore[IX(start.x, start.y, N)] = 0;
    heap_insert(&heap, start, h(start, end));
    while (heap.size > 0)
    {
        cell_t curr = heap_extract_min(&heap);
        int x = curr.x, y = curr.y;
        maze->walls[IX(x, y, N)] |= VISITED;

        for (int dir = 0; dir < 4; ++dir)
        {
            int dx = ddirs[dir][0], dy = ddirs[dir][1];
            int nx = x + dx, ny = y + dy;

            if (nx < 0 || ny < 0 || nx >= N || ny >= N)
                continue;

            int idx = IX(nx, ny, N);
            if (maze->walls[idx] & (VISITED | WALL))
                continue;

            if (maze->walls[idx] & END)
                heap.size = 0;

            if (gscore[idx] > gscore[IX(x, y, N)] + 1)
                gscore[idx] = gscore[IX(x, y, N)] + 1;
            cell_t next = (cell_t){ nx, ny };
            heap_insert(&heap, next, gscore[idx] + h(next, end));
        }
    }

    free_heap(&heap);
}

void astar_step(maze_t *maze)
{
    if (heap.size <= 0)
        return;

    int N = maze->N;
    cell_t curr = heap_extract_min(&heap);
    int x = curr.x, y = curr.y;

    int cidx = IX(x, y, N);
    maze->walls[cidx] |= VISITED;

    if (maze->walls[cidx] & END)
    {
        heap.size = 0;
        return;
    }

    for (int dir = 0; dir < 4; ++dir)
    {
        int dx = ddirs[dir][0], dy = ddirs[dir][1];
        int nx = x + dx, ny = y + dy;

        if (nx < 0 || ny < 0 || nx >= N || ny >= N)
            continue;

        int idx = IX(nx, ny, N);
        if (maze->walls[idx] & (VISITED | WALL))
            continue;

        int tentative_g = gscore[cidx] + 1;
        if (tentative_g < gscore[idx])
        {
            gscore[idx] = tentative_g;
            cell_t next = (cell_t){ nx, ny };
            heap_insert(&heap, next, gscore[idx] + h(next, end));
        }
    }
}

