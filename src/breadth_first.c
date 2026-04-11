
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/breadth_first.h"


void bf_search(maze_t *maze)
{
    int N = maze->N;
    cell_t *queue = get_collection();

    maze->walls[IX(1, 0, N)] |= VISITED;

    queue[0] = (cell_t){ 1, 0 };
    int queue_start = 0, queue_end = 1;
    while (queue_start < queue_end)
    {
        cell_t curr = queue[queue_start];
        int x = curr.x, y = curr.y;
        ++queue_start;

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

            queue[queue_end++] = (cell_t){ nx, ny };
        }
    }
}

