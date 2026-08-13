
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "include/graphics.h"
#include "include/heuristic.h"
#include "include/astar.h"
#include "include/breadth_first.h"
#include "include/depth_first.h"
#include "include/shader.h"


typedef struct
{
    const char *label;
    int id;
} menu_item_t;

typedef enum
{
    MENU_MAIN,
    MENU_HEURISTIC,
    MENU_CUSTOM
} menu_kind_t;

typedef enum
{
    TOOL_WALL,
    TOOL_ERASE,
    TOOL_START,
    TOOL_END
} custom_tool_t;

#define ACT_RUN_BFS         1
#define ACT_RUN_DFS         2
#define ACT_RUN_ASTAR       3
#define ACT_NEW_MAZE        4
#define ACT_CUSTOM_MAZE     5
#define ACT_CLEAR           6

#define ACT_H_MANHATTAN         101
#define ACT_H_EUCLIDEAN         102
#define ACT_H_SQUARED_EUCLIDEAN 103
#define ACT_H_CHEBYSHEV         104
#define ACT_H_COSINE            105
#define ACT_H_OCTILE            106

#define ACT_DRAW_START      111
#define ACT_DRAW_GOAL       112
#define ACT_DRAW_WALL       113
#define ACT_DRAW_ERASE      114
#define ACT_DRAW_CLEAR      115
#define ACT_DRAW_FINISH     116

#define ACT_BACK            199

typedef struct
{
    int open;
    double x, y;
    float w, h;
    int hover;
    int count;
    menu_kind_t kind;
    menu_item_t items[8];
} menu_t;

typedef struct
{
    GLuint texture;
    int width, height;
    int bearingX, bearingY;
    unsigned int advance;
} glyph_t;


#define BF      1
#define DF      2
#define ASTAR   3
#define CUSTOM  4

static const float quad[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

static volatile int fb_width = 1080, fb_height = 720;
static volatile int win_width = 1080, win_height = 720;
static volatile double mx = 0, my = 0;

static menu_t menu = {
    .open = 0,
    .w = 180.0f,
    .h = 28.0f,
    .hover = -1,
    .count = 6,
    .kind = MENU_MAIN,
    .items = {
        { "Run BFS",        1 },
        { "Run DFS",        2 },
        { "Run A*",         3 },
        { "New Maze",       4 },
        { "Custom Maze",    5 },
        { "Clear",          6 },
    }
};
static glyph_t glyphs[128];

static cell_t *collection;
int size, done = 0;
static GLuint vao, vbo;
static GLuint textVAO, textVBO;
static program_t mazeShader;
static program_t textShader;

static GLint uOffsetLoc, uScaleLoc, uColorLoc;
static GLint textProjLoc, textColorLoc;

static maze_t maze, copy;
static custom_tool_t custom_tool = TOOL_WALL;
static step alg = bf_step;
volatile heuristic h = manhattan_dist;
static int mode = BF;
static int custom_mode = 0;
static int N;
static int sx = 1, sy = 0;
static int ex, ey;


static int screen_to_cell(double mx, double my, int *cx, int *cy)
{
    if (mx < 0 || my < 0 || mx >= win_width || my >= win_height)
        return 0;

    int x = (int)(mx * copy.N / win_width);
    int y = (int)(my * copy.N / win_height);

    if (x < 0 || y < 0 || x >= copy.N || y >= copy.N)
        return 0;

    *cx = x;
    *cy = y;
    return 1;
}

static void apply_custom_tool(int x, int y)
{
    if (!custom_mode)
        return;

    int N = maze.N;
    int idx = IX(x, y, N);

    switch (custom_tool)
    {
        case TOOL_WALL:
            if (!(maze.walls[idx] & START) && !(maze.walls[idx] & END))
                maze.walls[idx] |= WALL;
            break;

        case TOOL_ERASE:
            maze.walls[idx] &= ~(WALL | VISITED);
            break;

        case TOOL_START:
            if (!(maze.walls[idx] & END))
            {
                clear_flag_everywhere(&maze, START);
                maze.walls[idx] &= ~WALL;
                maze.walls[idx] |= START;
                sx = x;
                sy = y;
            }
            break;

        case TOOL_END:
            if (!(maze.walls[idx] & START))
            {
                clear_flag_everywhere(&maze, END);
                maze.walls[idx] &= ~WALL;
                maze.walls[idx] |= END;
                ex = x;
                ey = y;
            }
            break;
    }

    free_maze(&copy);
    copy_maze(&maze, &copy);
}

static void draw_scene()
{
    int N = copy.N;
    float cellW = 2.0f / (float)N;
    float cellH = 2.0f / (float)N;

    float pad = 0.1f;
    float innerW = cellW * (1.0f - pad);
    float innerH = cellH * (1.0f - pad);
    float offsetX = (cellW - innerW) * 0.5f;
    float offsetY = (cellH - innerH) * 0.5f;

    glUseProgram(mazeShader);
    glBindVertexArray(vao);
    for (int y = 0; y < N; ++y)
    {
        for (int x = 0; x < N; ++x)
        {
            int8_t info = copy.walls[IX(x, y, N)];

            float ox = -1.0f + x * cellW + offsetX;
            float oy =  1.0f - (y + 1) * cellH + offsetY;

            float r = 1.0f, g = 1.0f, b = 1.0f;

            if (info & WALL)
            {
                r = 0.0f;
                g = 0.0f;
                b = 0.0f;
            } else if (info & START)
            {
                r = 0.0f;
                g = 0.0f;
                b = 1.0f;
            } else if (info & END)
            {
                r = 1.0f;
                g = 0.0f;
                b = 0.0f;
            } else if (info & (PATH_CURRENT | PATH_FINAL))
            {
                r = 0.0f;
                g = 1.0f;
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
            glUniform2f(uScaleLoc, innerW, innerH);
            glUniform3f(uColorLoc, r, g, b);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
}

static void update()
{
    if (done)
    {
        if (mode == BF)
            bf_clean();
        if (mode == DF)
            df_clean();
        if (mode == ASTAR)
            astar_clean();
        return;
    }

    alg(&copy);
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

static int menu_hit_test(double mx, double my)
{
    if (!menu.open)
        return -1;

    double x0 = menu.x;
    double y0 = menu.y;
    double x1 = x0 + menu.w;
    double y1 = y0 + menu.h * menu.count;

    if (mx < x0 || mx > x1 || my < y0 || my > y1)
        return -1;

    int idx = (int)((my - y0) / menu.h);
    if (idx < 0 || idx >= menu.count)
        return -1;

    return idx;
}

static int PAUSE = 1;
static void switch_to_bf()
{
    PAUSE = 1;
    mode = BF;
    dir_size = 4;
    copy_maze(&maze, &copy);
    init_bf(&copy, sx, sy);
    alg = bf_step;
    menu.open = 0;
}

static void switch_to_df()
{
    PAUSE = 1;
    mode = DF;
    dir_size = 4;
    copy_maze(&maze, &copy);
    init_df(&copy, sx, sy);
    alg = df_step;
    menu.open = 0;
}

static void switch_to_astar()
{
    PAUSE = 1;
    mode = ASTAR;
    dir_size = 4;
    copy_maze(&maze, &copy);
    init_astar(&copy, sx, sy, ex, ey);
    alg = astar_step;
}

static void make_new()
{
    PAUSE = 1;
    free_maze(&maze);
    free_maze(&copy);

    init_maze(&maze, N, sx, sy, ex, ey);
    copy_maze(&maze, &copy);
    if (mode == BF)
        init_bf(&copy, sx, sy);
    if (mode == DF)
        init_df(&copy, sx, sy);
    if (mode == ASTAR)
        init_astar(&copy, sx, sy, ex, ey);
    menu.open = 0;
}

static void create_custom_maze()
{
    PAUSE = 1;
    mode = CUSTOM;
    custom_mode = 1;

    copy_maze(&maze, &copy);
    menu.open = 0;
}

static void clear_maze()
{
    PAUSE = 1;
    copy_maze(&maze, &copy);
    if (mode == BF)
        init_bf(&copy, sx, sy);
    if (mode == DF)
        init_df(&copy, sx, sy);
    if (mode == ASTAR)
        init_astar(&copy, sx, sy, ex, ey);
    menu.open = 0;
}

static void open_main_menu(double x, double y)
{
    menu.open = 1;
    menu.kind = MENU_MAIN,
    menu.x = x;
    menu.y = y;
    menu.w = 180.0f;
    menu.h = 28.0f;
    menu.hover = -1;
    menu.count = 6;

    menu.items[0] = (menu_item_t){ "Run BFS",       ACT_RUN_BFS };
    menu.items[1] = (menu_item_t){ "Run DFS",       ACT_RUN_DFS };
    menu.items[2] = (menu_item_t){ "Run A*",        ACT_RUN_ASTAR };
    menu.items[3] = (menu_item_t){ "New Maze",      ACT_NEW_MAZE };
    menu.items[4] = (menu_item_t){ "Custom Maze",   ACT_CUSTOM_MAZE };
    menu.items[5] = (menu_item_t){ "Clear",         ACT_CLEAR };

    float total_h = menu.h * menu.count;
    if (menu.x + menu.w > win_width)
        menu.x = win_width - menu.w;
    if (menu.y + total_h > win_height)
        menu.y = win_height - menu.h;
    if (menu.x < 0)
        menu.x = 0;
    if (menu.y < 0)
        menu.y = 0;
}

static void open_heuristic_menu(double x, double y)
{
    menu.open = 1;
    menu.kind = MENU_HEURISTIC;
    menu.x = x;
    menu.y = y;
    menu.w = 220.0f;
    menu.h = 28.0f;
    menu.hover = -1;
    menu.count = 7;

    menu.items[0] = (menu_item_t){ "Manhattan",         ACT_H_MANHATTAN };
    menu.items[1] = (menu_item_t){ "Euclidean",         ACT_H_EUCLIDEAN };
    menu.items[2] = (menu_item_t){ "Squared Euclidean", ACT_H_EUCLIDEAN };
    menu.items[3] = (menu_item_t){ "Chebyshev",         ACT_H_CHEBYSHEV };
    menu.items[4] = (menu_item_t){ "Cosine",            ACT_H_COSINE };
    menu.items[5] = (menu_item_t){ "Octile",            ACT_H_OCTILE };
    menu.items[6] = (menu_item_t){ "Back",              ACT_BACK };

    float total_h = menu.h * menu.count;
    if (menu.x + menu.w > win_width)
        menu.x = win_width - menu.w;
    if (menu.y + total_h > win_height)
        menu.y = win_height - menu.h;
    if (menu.x < 0)
        menu.x = 0;
    if (menu.y < 0)
        menu.y = 0;
}

static void open_custom_menu(double x, double y)
{
    menu.open = 1;
    menu.kind = MENU_HEURISTIC;
    menu.x = x;
    menu.y = y;
    menu.w = 220.0f;
    menu.h = 28.0f;
    menu.hover = -1;
    menu.count = 6;

    menu.items[0] = (menu_item_t){ "Start",     ACT_DRAW_START };
    menu.items[1] = (menu_item_t){ "Goal",      ACT_DRAW_GOAL };
    menu.items[2] = (menu_item_t){ "Wall",      ACT_DRAW_WALL };
    menu.items[3] = (menu_item_t){ "Erase",     ACT_DRAW_ERASE };
    menu.items[4] = (menu_item_t){ "Clear",     ACT_DRAW_CLEAR };
    menu.items[5] = (menu_item_t){ "Finish",    ACT_DRAW_FINISH };

    float total_h = menu.h * menu.count;
    if (menu.x + menu.w > win_width)
        menu.x = win_width - menu.w;
    if (menu.y + total_h > win_height)
        menu.y = win_height - menu.h;
    if (menu.x < 0)
        menu.x = 0;
    if (menu.y < 0)
        menu.y = 0;
}

static void do_menu_action(int id)
{
    switch (id)
    {
        case ACT_RUN_BFS:
            switch_to_bf();
            break;

        case ACT_RUN_DFS:
            switch_to_df();
            break;

        case ACT_RUN_ASTAR:
            open_heuristic_menu(menu.x + menu.w + 4.0, menu.y);
            break;

        case ACT_NEW_MAZE:
            make_new();
            break;

        case ACT_CUSTOM_MAZE:
            create_custom_maze();
            break;

        case ACT_CLEAR:
            clear_maze();
            break;

        case ACT_H_MANHATTAN:
            h = manhattan_dist;
            switch_to_astar();
            menu.open = 0;
            break;

        case ACT_H_EUCLIDEAN:
            h = euclidean_dist;
            switch_to_astar();
            dir_size = 8;
            menu.open = 0;
            break;

        case ACT_H_SQUARED_EUCLIDEAN:
            h = squared_euclidean_dist;
            switch_to_astar();
            dir_size = 8;
            menu.open = 0;
            break;

        case ACT_H_CHEBYSHEV:
            h = chebyshev_dist;
            switch_to_astar();
            menu.open = 0;
            break;

        case ACT_H_COSINE:
            h = cosine_dist;
            switch_to_astar();
            dir_size = 8;
            menu.open = 0;
            break;

        case ACT_H_OCTILE:
            h = octile_dist;
            switch_to_astar();
            dir_size = 8;
            menu.open = 0;
            break;

        case ACT_DRAW_START:
            custom_tool = TOOL_START;
            menu.open = 0;
            break;

        case ACT_DRAW_GOAL:
            custom_tool = TOOL_END;
            menu.open = 0;
            break;

        case ACT_DRAW_WALL:
            custom_tool = TOOL_WALL;
            menu.open = 0;
            break;

        case ACT_DRAW_ERASE:
            custom_tool = TOOL_ERASE;
            menu.open = 0;
            break;

        case ACT_DRAW_CLEAR:
            int N = maze.N;
            for (int i = 0; i < N*N; ++i)
                maze.walls[i] = OPEN;

            sx = 0; sy = 0;
            ex = N-1; ey = N-1;
            maze.walls[IX(sx, sy, N)] = START;
            maze.walls[IX(ex, ey, N)] = END;

            free_maze(&copy);
            copy_maze(&maze, &copy);
            menu.open = 0;
            break;

        case ACT_DRAW_FINISH:
            custom_mode = 0;
            copy_maze(&maze, &copy);
            switch_to_bf();
            menu.open = 0;
            break;

        case ACT_BACK:
            open_main_menu(menu.x, menu.y);
            break;
    }
}

static int HOLDING = 0;
static int LEFT_DOWN = 0;
static void process_input(GLFWwindow *window)
{
    int ESC = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    int SPACE = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    int CK = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    int SK = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    int NK = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
    int BK = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    int DK = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    int AK = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    int K1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    int K2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    int K3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    int K4 = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
    int ENTER = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

    if (!HOLDING && ESC)
        glfwSetWindowShouldClose(window, true);

    if (!HOLDING && CK)
    {
        HOLDING = 1;
        clear_maze();
    }

    if (!HOLDING && SK)
    {
        HOLDING = 1;
        update();
    }

    if (!HOLDING && NK)
    {
        HOLDING = 1;
        make_new();
    }

    if (!HOLDING && BK)
    {
        HOLDING = 1;
        switch_to_bf();
    }

    if (!HOLDING && DK)
    {
        HOLDING = 1;
        switch_to_df();
    }

    if (!HOLDING && AK)
    {
        HOLDING = 1;
        switch_to_astar();
    }

    if (!HOLDING && custom_mode && K1)
    {
        HOLDING = 1;
        custom_tool = TOOL_WALL;
    }

    if (!HOLDING && custom_mode && K2)
    {
        HOLDING = 1;
        custom_tool = TOOL_ERASE;
    }

    if (!HOLDING && custom_mode && K3)
    {
        HOLDING = 1;
        custom_tool = TOOL_START;
    }

    if (!HOLDING && custom_mode && K4)
    {
        HOLDING = 1;
        custom_tool = TOOL_END;
    }

    if (!HOLDING && custom_mode && ENTER)
    {
        HOLDING = 1;
        custom_mode = 0;

        free_maze(&copy);
        switch_to_bf();
    }

    if (!HOLDING && SPACE)
    {
        HOLDING = 1;
        PAUSE = !PAUSE;
    }

    if (!ESC &&
        !CK && !SK && !NK && !BK && !DK && !AK &&
        !K1 && !K2 && !K3 && !K4 && !ENTER &&
        !SPACE)
        HOLDING = 0;
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window;
    menu.hover = menu_hit_test(xpos, ypos);

    if (custom_mode && LEFT_DOWN)
    {
        int cx, cy;
        if (screen_to_cell(xpos, ypos, &cx, &cy))
            apply_custom_tool(cx, cy);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        if (custom_mode)
        {
            open_custom_menu(mx, my);
            menu.hover = menu_hit_test(mx, my);
            return;
        }

        open_main_menu(mx, my);
        menu.hover = menu_hit_test(mx, my);
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (custom_mode && action == GLFW_RELEASE)
        {
            LEFT_DOWN = 0;
            return;
        }

        if (action == GLFW_PRESS)
        {
            int hit = menu_hit_test(mx, my);
            if (hit >= 0)
            {
                do_menu_action(menu.items[hit].id);
                return;
            }
            else
                menu.open = 0;
        }

        if (custom_mode && action == GLFW_PRESS)
        {
            int cx, cy;
            if (screen_to_cell(mx, my, &cx, &cy))
                apply_custom_tool(cx, cy);
            LEFT_DOWN = 1;
            return;
        }
    }
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    PAUSE = 1;
    if (yoffset > 0.0)
        N += 2;
    else
        N -= 2;

    sx = 1;
    sy = 0;
    ex = N - 1;
    ey = N - 2;
    free_maze(&maze);
    if (init_maze(&maze, N, sx, sy, ex, ey))
        return;

    if (collection)
    {
        free(collection);
        collection = NULL;
    }

    collection = (cell_t *)malloc(N*N * sizeof(cell_t));
    if (!collection)
    {
        perror("Allocating collection");
        return;
    }

    size = N*N;
    free_maze(&copy);
    switch_to_bf();
}

static float to_ndc_x(float x)
{
    return (2.0f * x / win_width) - 1.0f;
}

static float to_ndc_y(float y)
{
    return 1.0f - (2.0f * y / win_height);
}

static void draw_rect(float x0, float y0, float x1, float y1)
{
    float nx0 = to_ndc_x(x0);
    float ny0 = to_ndc_y(y0);
    float nx1 = to_ndc_x(x1);
    float ny1 = to_ndc_y(y1);

    glBegin(GL_QUADS);
    glVertex2f(nx0, ny0);
    glVertex2f(nx1, ny0);
    glVertex2f(nx1, ny1);
    glVertex2f(nx0, ny1);
    glEnd();
}

static void render_text(const char *text, float x, float y, float scale, float r, float g, float b)
{
    glUseProgram(textShader);
    glUniform3f(textColorLoc, r, g, b);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    for (const char *p = text; *p; ++p)
    {
        unsigned char c = (unsigned char)*p;
        glyph_t ch = glyphs[c];

        float xpos = x + ch.bearingX * scale;
        float ypos = y + (ch.height - ch.bearingY) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;

        float vertices[6][4] = {
            { xpos, ypos + h,       0.0f, 1.0f },
            { xpos, ypos,           0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },

            { xpos, ypos + h,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void make_ortho(float left, float right, float bottom, float top, float out[16])
{
    for (int i = 0; i < 16; ++i)
        out[i] = 0.0f;

    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -1.0f;
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[15] = 1.0f;
}

static void draw_menu(void)
{
    if (!menu.open)
        return;


    for (int i = 0; i < menu.count; ++i)
    {
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        float x0 = (float)menu.x;
        float y0 = (float)menu.y + i * menu.h;
        float x1 = x0 + menu.w;
        float y1 = y0 + menu.h;

        if (i == menu.hover)
            glColor4f(0.35f, 0.35f, 0.35f, 0.95f);
        else
            glColor4f(0.20f, 0.20f, 0.20f, 0.75f);

        draw_rect(x0, y0, x1, y1);

        float scale = 0.4f;
        render_text(menu.items[i].label, x0 + 8.0f, (y1 + y0) / 2.0f, scale, 1.0f, 1.0f, 1.0f);
    }
}

static int init_text(const char *font_path)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "Failed to init FreeType\n");
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path, 0, &face))
    {
        fprintf(stderr, "Failed to load font: %s\n", font_path);
        FT_Done_FreeType(ft);
        return 1;
    }

    FT_Set_Pixel_Sizes(face, 0, 24);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            fprintf(stderr, "Failed to load glyph %d\n", c);
            continue;
        }

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glyphs[c].texture   = tex;
        glyphs[c].width     = face->glyph->bitmap.width;
        glyphs[c].height    = face->glyph->bitmap.rows;
        glyphs[c].bearingX  = face->glyph->bitmap_left;
        glyphs[c].bearingY  = face->glyph->bitmap_top;
        glyphs[c].advance   = (unsigned int)face->glyph->advance.x;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}

static void init_text_gl(void)
{
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLFWwindow *init_graphics()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "OpenGL Pathfinding App", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        glfwDestroyWindow(window);
        glfwTerminate();
        return NULL;
    }

    glGetError();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (init_text("fonts/FiraCodeNerdFontMono-Bold.ttf"))
    {
        fprintf(stderr, "Could not initialize fonts.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return NULL;
    }
    init_text_gl();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int n = 25;
    N = 2*n + 1;
    size = N * N;
    ex = N - 1;
    ey = N - 2;
    if (init_maze(&maze, N, sx, sy, ex, ey))
        return NULL;

    collection = (cell_t *)malloc(size * sizeof(cell_t));
    if (!collection)
    {
        perror("Allocating collection");
        return NULL;
    }

    mazeShader = init_shader("shaders/vertex/quad.vert", "shaders/fragment/quad.frag");
    textShader = init_shader("shaders/vertex/text.vert", "shaders/fragment/text.frag");

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

    textProjLoc  = glGetUniformLocation(textShader, "projection");
    textColorLoc = glGetUniformLocation(textShader, "textColor");
    float proj[16];
    make_ortho(0.0f, (float)win_width, (float)win_height, 0.0f, proj);
    glUseProgram(textShader);
    glUniformMatrix4fv(textProjLoc, 1, GL_FALSE, proj);

    int mode = BF;
    copy_maze(&maze, &copy);
    init_bf(&copy, sx, sy);
    return window;
}

void free_graphics(GLFWwindow *window)
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

    free_maze(&maze);
    glDeleteProgram(mazeShader);
    glfwDestroyWindow(window);
    glfwTerminate();
}

cell_t *get_collection()
{
    return collection;
}

void set_collection(cell_t *coll)
{
    collection = coll;
}

void draw_loop(GLFWwindow *window)
{
    while (!glfwWindowShouldClose(window))
    {
        process_input(window);

        glViewport(0, 0, fb_width, fb_height);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!PAUSE)
            update();
        draw_scene();
        draw_menu();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_graphics(window);
}

