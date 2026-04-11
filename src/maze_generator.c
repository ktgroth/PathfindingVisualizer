
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/maze_generator.h"


int init_maze(maze_t *maze, size_t N)
{
    if (!maze || N < 3 || N % 2 == 0)
        return 1;

    maze->N = N;
    maze->walls = (int8_t *)calloc(4*N*N, sizeof(int8_t));
    if (!maze->walls)
    {
        perror("Allocation of maze walls");
        return 1;
    }

    cell_t *stack = (cell_t *)malloc(4*N*N * sizeof(cell_t));
    if (!stack)
    {
        perror("Generating maze paths");
        free_maze(maze);
        return 1;
    }

    srand((unsigned)time(NULL));
    stack[0] = (cell_t){ 1, 1 };
    int stack_top = 0;

    maze->walls[IX(1, 0, N)] = OPEN;
    maze->walls[IX(1, 1, N)] = OPEN;
    maze->walls[IX(N - 1, N - 2, N)] = OPEN;
    while (stack_top >= 0)
    {
        int x = stack[stack_top].x, y = stack[stack_top].y;

        int possible[4];
        int count = 0;
        for (int dir = 0; dir < 4; ++dir)
        {
            int dx = dirs[dir][0], dy = dirs[dir][1];
            int nx = x + dx, ny = y + dy;

            if (nx <= 0 || nx >= N - 1 || ny <= 0 || ny >= N - 1)
                continue;

            if (maze->walls[IX(nx, ny, N)] == OPEN)
                continue;

            possible[count++] = dir;
        }

        if (count == 0)
        {
            --stack_top;
            continue;
        }

        int dir = possible[rand() % count];
        int dx = dirs[dir][0], dy = dirs[dir][1];
        int wx = x + dx / 2, wy = y + dy / 2;
        int nx = x + dx, ny = y + dy;

        maze->walls[IX(wx, wy, N)] = OPEN;
        maze->walls[IX(nx, ny, N)] = OPEN;
        stack[++stack_top] = (cell_t){ nx, ny };
    }

    free(stack);
    return 0;
}

void free_maze(maze_t *maze)
{
    if (!maze)
        return;

    free(maze->walls);
    maze->N = 0;
}

