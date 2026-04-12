
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "include/graphics.h"
#include "include/shader.h"


static const float quad[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

volatile int fb_width = 1080, fb_height = 720;
volatile int win_width = 1080, win_height = 720;
volatile double mx = 0, my = 0;

static cell_t *collection;
static GLuint vao, vbo;
static program_t mazeShader;

static GLint uOffsetLoc, uScaleLoc, uColorLoc;
static int gridN;


int init_graphics(int N)
{
    gridN = N;
    collection = (cell_t *)malloc(N*N * sizeof(cell_t *));
    if (!collection)
    {
        perror("Allocating collection");
        return 1;
    }

    mazeShader = init_shader("shaders/vertex/quad.vert", "shaders/fragment/quad.frag");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glUseProgram(mazeShader);

    uOffsetLoc  = glGetUniformLocation(mazeShader, "uOffset");
    uScaleLoc   = glGetUniformLocation(mazeShader, "uScale");
    uColorLoc   = glGetUniformLocation(mazeShader, "uColor");

    return 0;
}

void free_graphics()
{
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (collection)
        free(collection);

    vbo = 0;
    vao = 0;
    collection = NULL;
}

cell_t *get_collection()
{
    return collection;
}

void draw(maze_t *maze)
{
    int N = maze->N;
    float cellW = 2.0f / (float)N;
    float cellH = 2.0f / (float)N;

    glUseProgram(mazeShader);
    glBindVertexArray(vao);
    for (int y = 0; y < N; ++y)
    {
        for (int x = 0; x < N; ++x)
        {
            int8_t info = maze->walls[IX(x, y, N)];

            float ox = -1.0f + x * cellW;
            float oy =  1.0f - (y + 1) * cellH;

            float r = 1.0f, g = 1.0f, b = 1.0f;

            if (info & WALL)
            {
                r = 0.0f;
                g = 0.0f;
                b = 0.0f;
            } else if (info & START)
            {
                r = 0.0f;
                g = 1.0f;
                b = 0.0f;
            } else if (info & END)
            {
                r = 1.0f;
                g = 0.0f;
                b = 0.0f;
            } else if (info & VISITED)
            {
                r = 1.0f;
                g = 0.94f;
                b = 0.0f;
            } else
            {
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
            }

            glUniform2f(uOffsetLoc, ox, oy);
            glUniform2f(uScaleLoc, cellW, cellH);
            glUniform3f(uColorLoc, r, g, b);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
}

void update(maze_t *maze, step fn)
{
    fn(maze);
}

