#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "config.h"
#include "game.h"
#include "renderer.h"
#include "sprites.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    Game    game;
    Sprites sprites;
} App;

static void key_callback(GLFWwindow *win, int key, int scancode,
                         int action, int mods)
{
    (void)scancode; (void)mods;
    if (action != GLFW_PRESS)
        return;

    App *app = glfwGetWindowUserPointer(win);
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(win, GLFW_TRUE);
        break;
    case GLFW_KEY_ENTER:
    case GLFW_KEY_KP_ENTER:
    case GLFW_KEY_SPACE:
        game_key(&app->game, KEY_START);
        break;
    case GLFW_KEY_R:
        game_key(&app->game, KEY_RESTART);
        break;
    case GLFW_KEY_M:
        game_key(&app->game, KEY_MENU);
        break;
    default:
        break;
    }
}

static void cursor_to_game(GLFWwindow *win, float *x, float *y)
{
    double cx, cy;
    int ww, wh;
    glfwGetCursorPos(win, &cx, &cy);
    glfwGetWindowSize(win, &ww, &wh);
    *x = (float)(cx * WIDTH / ww);
    *y = (float)(HEIGHT - cy * HEIGHT / wh);
}

static void mouse_callback(GLFWwindow *win, int button, int action, int mods)
{
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    App *app = glfwGetWindowUserPointer(win);
    float x, y;
    cursor_to_game(win, &x, &y);
    game_click(&app->game, x, y, &app->sprites);
}

static int pressed(GLFWwindow *win, int a, int b)
{
    return glfwGetKey(win, a) == GLFW_PRESS || glfwGetKey(win, b) == GLFW_PRESS;
}

int main(void)
{
    static App app;

    srand((unsigned)time(NULL));

    if (!glfwInit()) {
        fprintf(stderr, "Falha ao inicializar GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *win = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!win) {
        fprintf(stderr, "Falha ao criar janela (OpenGL 3.3 disponivel?)\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Falha ao carregar OpenGL com glad\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    printf("OpenGL %s\n", (const char *)glGetString(GL_VERSION));

    glfwSwapInterval(1);   /* vsync */
    glfwSetWindowUserPointer(win, &app);
    glfwSetKeyCallback(win, key_callback);
    glfwSetMouseButtonCallback(win, mouse_callback);

    if (!renderer_init() || !sprites_load(&app.sprites)) {
        fprintf(stderr, "Falha ao iniciar graficos ou carregar sprites\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    game_init(&app.game);

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(win)) {
        
        double now = glfwGetTime();
        float  dt  = (float)(now - last);
        last = now;
        if (dt > 0.05f)
            dt = 0.05f;

        glfwPollEvents();

        Input in = {
            .left  = pressed(win, GLFW_KEY_LEFT,  GLFW_KEY_A),
            .right = pressed(win, GLFW_KEY_RIGHT, GLFW_KEY_D),
            .up    = pressed(win, GLFW_KEY_UP,    GLFW_KEY_W),
            .down  = pressed(win, GLFW_KEY_DOWN,  GLFW_KEY_S),
        };
        cursor_to_game(win, &app.game.mouse_x, &app.game.mouse_y);

        game_update(&app.game, &in, dt);

        int fbw, fbh;
        glfwGetFramebufferSize(win, &fbw, &fbh);
        renderer_begin(fbw, fbh, 0.02f, 0.02f, 0.08f);
        game_render(&app.game, &app.sprites);
        renderer_end();

        glfwSwapBuffers(win);
    }

    sprites_free(&app.sprites);
    renderer_shutdown();
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
