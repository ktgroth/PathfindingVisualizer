#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H


#include <stdint.h>


#define WALL        0
#define OPEN        1
#define UNVISITED   1
#define VISITED     2

#define IX(x, y, N) ((y) * (4*N) + (x))


static int8_t dirs[4][2]  = { { 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 } };
static int8_t ddirs[8][2] = 
    { {  1,  0 }, { -1,  0 }, {  0,  1 }, {  0, -1 },
      {  1,  1 }, { -1,  1 }, {  1, -1 }, { -1, -1 } };


typedef struct
{
    size_t  N;
    int8_t  *walls;
} maze_t;

typedef struct
{
    int x, y;
} cell_t;


int init_maze(maze_t *maze, size_t N);
void free_maze(maze_t *maze);

#endif

