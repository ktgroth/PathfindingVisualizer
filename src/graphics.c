
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

typedef struct
{
    int open;
    double x, y;
    float w, h;
    int hover;
    int count;
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
static GLuint vao, vbo;
static GLuint textVAO, textVBO;
static program_t mazeShader;
static program_t textShader;

static GLint uOffsetLoc, uScaleLoc, uColorLoc;
static GLint textProjLoc, textColorLoc;

static maze_t maze, copy;
static step alg = bf_step;
volatile heuristic h = manhattan_dist;
static int mode = BF;
static int N;
static int sx = 1, sy = 0;
static int ex, ey;


static void draw_scene()
{
    int N = copy.N;
    float cellW = 2.0f / (float)N;
    float cellH = 2.0f / (float)N;

    glUseProgram(mazeShader);
    glBindVertexArray(vao);
    for (int y = 0; y < N; ++y)
    {
        for (int x = 0; x < N; ++x)
        {
            int8_t info = copy.walls[IX(x, y, N)];

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

static void update()
{
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
    copy_maze(&maze, &copy);
    init_bf(&copy, sx, sy);
    alg = bf_step;
}

static void switch_to_df()
{
    PAUSE = 1;
    mode = DF;
    copy_maze(&maze, &copy);
    init_df(&copy, sx, sy);
    alg = df_step;
}

static void switch_to_astar()
{
    PAUSE = 1;
    mode = ASTAR;
    copy_maze(&maze, &copy);
    init_astar(&copy, sx, sy, ex, ey);
    alg = astar_step;
}

static void make_new()
{
    PAUSE = 1;
    free_maze(&maze);
    free_maze(&copy);

    init_maze(&maze, N);
    copy_maze(&maze, &copy);
    if (mode == BF)
        init_bf(&copy, sx, sy);
    if (mode == DF)
        init_df(&copy, sx, sy);
    if (mode == ASTAR)
        init_astar(&copy, sx, sy, ex, ey);
}

static void create_custom_maze()
{
    PAUSE = 1;

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
}

static void do_menu_action(int id)
{
    switch (id)
    {
        case 1:
            printf("Run BFS\n");
            switch_to_bf();
            break;

        case 2:
            printf("Run DFS\n");
            switch_to_df();
            break;

        case 3:
            printf("Run A*\n");
            switch_to_astar();
            break;

        case 4:
            printf("New Maze\n");
            make_new();
            break;

        case 5:
            printf("Custom Maze\n");
            create_custom_maze();
            break;

        case 6:
            printf("Clear\n");
            clear_maze();
            break;
    }
}

static int HOLDING = 0;
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

    if (!HOLDING && SPACE)
    {
        HOLDING = 1;
        PAUSE = !PAUSE;
    }

    if (!ESC && !CK && !SK && !NK && !BK && !DK && !AK && !SPACE)
        HOLDING = 0;
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window;
    menu.hover = menu_hit_test(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    if (action != GLFW_PRESS)
        return;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        menu.open = 1;
        menu.x = mx;
        menu.y = my;

        float total_h = menu.h * menu.count;
        if (menu.x + menu.w > win_width)
            menu.x = win_width - menu.w;
        if (menu.y + total_h > win_height)
            menu.y = win_height - total_h;
        if (menu.x < 0)
            menu.x = 0;
        if (menu.y < 0)
            menu.y = 0;

        menu.hover = menu_hit_test(mx, my);
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        int hit = menu_hit_test(mx, my);
        if (hit >= 0)
            do_menu_action(menu.items[hit].id);
        menu.open = 0;
    }
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
    ex = N - 1;
    ey = N - 2;
    if (init_maze(&maze, N))
        return NULL;

    collection = (cell_t *)malloc(N*N * sizeof(cell_t));
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
}

