#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H


#include <stdint.h>


#define WALL        0
#define OPEN        1
#define UNVISITED   1
#define VISITED     2

#define IX(x, y, N) ((y) * (4*N) + (x))


typedef struct
{
    size_t  N;
    int8_t  *walls;
} maze_t;


int init_maze(maze_t *maze, size_t N);
void free_maze(maze_t *maze);

#endif

