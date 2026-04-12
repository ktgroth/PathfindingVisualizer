
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "include/graphics.h"
#include "include/maze_generator.h"
#include "include/breadth_first.h"
#include "include/depth_first.h"


static maze_t maze, copy;
static step alg = bf_step;
static int mode = BF;
static int sx = 1, sy = 0;
static int N;


static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window;
    mx = xpos;
    my = ypos;
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    fb_width = width;
    fb_height = height;
    glViewport(0, 0, width, height);
}

static void window_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    win_width = width;
    win_height = height;
}

static int HOLDING = 0;
static int PAUSE = 1;
static void process_input(GLFWwindow *window)
{
    int ESC = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    int SPACE = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    int SK = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    int NK = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
    int BK = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    int DK = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    if (!HOLDING && ESC)
        glfwSetWindowShouldClose(window, true);

    if (!HOLDING && SK)
    {
        HOLDING = 1;
        update(&copy, alg);
    }

    if (!HOLDING && NK)
    {
        HOLDING = 1;
        free_maze(&maze);
        free_maze(&copy);

        init_maze(&maze, N);
        copy_maze(&maze, &copy);
        if (mode == BF)
            init_bf(&copy, sx, sy);
        if (mode == DF)
            init_df(&copy, sx, sy);
    }

    if (!HOLDING && BK)
    {
        HOLDING = 1;
        mode = BF;
        copy_maze(&maze, &copy);
        init_bf(&copy, sx, sy);
        alg = bf_step;
    }

    if (!HOLDING && DK)
    {
        HOLDING = 1;
        mode = DF;
        copy_maze(&maze, &copy);
        init_df(&copy, sx, sy);
        alg = df_step;
    }

    if (!HOLDING && SPACE)
    {
        HOLDING = 1;
        PAUSE = !PAUSE;
    }

    if (!ESC && !SK && !NK && !BK && !DK && !SPACE)
        HOLDING = 0;
}

void print_maze(maze_t *maze)
{
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            int8_t info = maze->walls[IX(j, i, N)];
            if (info & WALL)
                printf("#");
            else if (info & (START | END))
                printf("@");
            else if (info & VISITED)
                printf("*");
            else
                printf(" ");
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "OpenGL Pathfinding App", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glGetError();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    int n = 25;
    N = 2*n + 1;
    if (init_maze(&maze, N))
        return 1;

    if (init_graphics(N))
        return 1;

    int mode = BF;
    copy_maze(&maze, &copy);
    init_bf(&copy, sx, sy);
    while (!glfwWindowShouldClose(window))
    {
        process_input(window);

        glViewport(0, 0, fb_width, fb_height);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!PAUSE)
            update(&copy, alg);
        draw(&copy);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_maze(&maze);
    free_graphics();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

