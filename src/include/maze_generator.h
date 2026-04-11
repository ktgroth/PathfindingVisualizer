#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H


#include <stdint.h>


#define WALL        0b00001
#define OPEN        0b00010
#define VISITED     0b00100
#define START       0b01000
#define END         0b10000

#define IX(x, y, N) ((y) * (N) + (x))


static int8_t dirs[4][2]  = { { 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 } };
static int8_t ddirs[8][2] =
    { {  1,  0 }, { -1,  0 }, {  0,  1 }, {  0, -1 },
      {  1,  1 }, { -1,  1 }, {  1, -1 }, { -1, -1 } };


typedef struct
{
    int N;
    int8_t  *walls;
} maze_t;

typedef struct
{
    int x, y;
} cell_t;


int init_maze(maze_t *maze, int N);
int copy_maze(maze_t *src, maze_t *dst);
void free_maze(maze_t *maze);

#endif

