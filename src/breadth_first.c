
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/breadth_first.h"


void bf_search(maze_t *maze)
{
    size_t N = maze->N;
    cell_t *queue = (cell_t *)malloc(N*N * sizeof(cell_t));
    if (!queue)
    {
        perror("Breadth First Search");
        return;
    }

    queue[0] = (cell_t){ 1, 0 };
    int queue_start = 0, queue_end = 1;

    while (queue_start < queue_end)
    {
        int x = queue[queue_start].x, y = queue[queue_start].y;
        ++queue_start;

        for (int dir = 0; dir < 4; ++dir)
        {
            int dx = ddirs[dir][0], dy = ddirs[dir][1];
            int nx = x + dx, ny = y + dy;

            if (nx == N - 1 && ny == N - 2)
            {
                maze->walls[IX(nx, ny, N)] = VISITED;
                queue_start = queue_end;
                break;
            }

            if (nx < 0 || ny < 0 || nx >= N || ny >= N)
                continue;

            if (maze->walls[IX(nx, ny, N)] != OPEN)
                continue;

            maze->walls[IX(nx, ny, N)] = VISITED;
            if (nx == N - 1 && ny == N - 2)
            {
                free(queue);
                return;
            }

            queue[queue_end++] = (cell_t){ nx, ny };
        }
    }

    free(queue);
}

