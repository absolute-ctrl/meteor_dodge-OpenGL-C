#include "game.h"
#include "renderer.h"
#include "text.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define START_SCALE 1.0f
#define START_Y     250.0f

static float randf(float a, float b)
{
    return a + (b - a) * ((float)rand() / (float)RAND_MAX);
}


static void reset_round(Game *g)
{
    g->player.x = WIDTH / 2.0f;
    g->player.y = 80.0f;
    for (int i = 0; i < MAX_METEORS; i++)
        g->meteors[i].active = 0;
    g->elapsed     = 0.0f;
    g->spawn_timer = 0.5f;
    g->points      = 0;
}

void game_init(Game *g)
{
    for (int i = 0; i < NUM_STARS; i++) {
        g->stars[i].x     = randf(0, WIDTH);
        g->stars[i].y     = randf(0, HEIGHT);
        g->stars[i].speed = randf(20, 80);
    }
    reset_round(g);
    g->state = MENU;
}

static void start_round(Game *g)
{
    reset_round(g);
    g->state = PLAYING;
}


static void spawn_meteor(Game *g)
{
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor *m = &g->meteors[i];
        if (m->active)
            continue;

        m->radius = randf(15.0f, 40.0f);
        m->x      = randf(m->radius, WIDTH - m->radius);
        m->y      = HEIGHT + m->radius;

        m->speed  = randf(120.0f, 220.0f) + g->elapsed * 4.0f;
        m->vx     = randf(-40.0f, 40.0f);
        m->angle  = 0.0f;
        m->spin   = randf(-90.0f, 90.0f);
        for (int k = 0; k < METEOR_SIDES; k++)
            m->shape[k] = randf(0.75f, 1.0f);
        m->active = 1;
        return;
    }
}


static int collides(const Player *p, const Meteor *m)
{
    float dx = m->x - p->x;
    float dy = m->y - p->y;
    float r  = m->radius * 0.85f + PLAYER_RADIUS;
    return dx * dx + dy * dy < r * r;
}

static void update_stars(Game *g, float dt)
{
    for (int i = 0; i < NUM_STARS; i++) {
        Star *s = &g->stars[i];
        s->y -= s->speed * dt;
        if (s->y < 0) {
            s->y += HEIGHT;
            s->x  = randf(0, WIDTH);
        }
    }
}

static void update_player(Player *p, const Input *in, float dt)
{
    float dx = (float)(in->right - in->left);
    float dy = (float)(in->up - in->down);

    if (dx != 0.0f && dy != 0.0f) {
        dx *= 0.7071f;
        dy *= 0.7071f;
    }

    p->x += dx * PLAYER_SPEED * dt;
    p->y += dy * PLAYER_SPEED * dt;

    if (p->x < 24.0f)          p->x = 24.0f;
    if (p->x > WIDTH - 24.0f)  p->x = WIDTH - 24.0f;
    if (p->y < 30.0f)          p->y = 30.0f;
    if (p->y > HEIGHT * 0.6f)  p->y = HEIGHT * 0.6f;
}

void game_update(Game *g, const Input *in, float dt)
{

    update_stars(g, dt);

    if (g->state != PLAYING)
        return;

    g->elapsed += dt;
    if (g->elapsed >= WIN_TIME) {
        g->elapsed = WIN_TIME;
        g->state   = VICTORY;
        printf("Vitoria! Pontos: %d\n", g->points);
        return;
    }

    update_player(&g->player, in, dt);


    g->spawn_timer -= dt;
    if (g->spawn_timer <= 0.0f) {
        spawn_meteor(g);
        g->spawn_timer = fmaxf(0.18f, 0.8f - g->elapsed * 0.011f);
    }

    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor *m = &g->meteors[i];
        if (!m->active)
            continue;

        m->y     -= m->speed * dt;
        m->x     += m->vx * dt;
        m->angle += m->spin * dt;

        if (m->y < -m->radius) {
            m->active = 0;
            g->points++;
            continue;
        }

        if (collides(&g->player, m)) {
            g->state = GAME_OVER;
            printf("Game over! Pontos: %d  Tempo: %.1f s\n",
                   g->points, g->elapsed);
            return;
        }
    }
}

void game_key(Game *g, GameKey key)
{
    switch (key) {
    case KEY_START:
        if (g->state == MENU)
            start_round(g);
        break;
    case KEY_RESTART:
        if (g->state == GAME_OVER || g->state == VICTORY)
            start_round(g);
        break;
    case KEY_MENU:
        if (g->state == GAME_OVER || g->state == VICTORY) {
            reset_round(g);
            g->state = MENU;
        }
        break;
    }
}

static int inside_start(const Sprites *sp, float x, float y)
{
    float w  = sp->start.w * START_SCALE;
    float h  = sp->start.h * START_SCALE;
    float bx = (WIDTH - w) / 2.0f;
    return x >= bx - 16 && x <= bx + w + 16 &&
           y >= START_Y - 12 && y <= START_Y + h + 12;
}

void game_click(Game *g, float x, float y, const Sprites *sp)
{
    if (g->state == MENU && inside_start(sp, x, y))
        start_round(g);
}

static void draw_stars(const Game *g)
{
    for (int i = 0; i < NUM_STARS; i++) {
        const Star *s = &g->stars[i];
        float b = s->speed / 80.0f;
        renderer_color(b, b, b, 1.0f);
        renderer_rect(s->x, s->y, 2, 2);
    }
}

static void draw_player(const Player *p, int flame)
{
    float x = p->x, y = p->y;

    if (flame) {
        renderer_color(1.0f, 0.55f, 0.1f, 1.0f);
        renderer_triangle(x - 5, y - 18, x + 5, y - 18,
                          x, y - 30 - randf(0.0f, 8.0f));
    }

    renderer_color(0.45f, 0.55f, 0.7f, 1.0f);
    renderer_triangle(x - 26, y - 8,  x + 26, y - 8,  x, y + 10);
    renderer_triangle(x - 11, y - 19, x + 11, y - 19, x, y - 8);

    float body[] = {
        x - 5, y - 18,
        x + 5, y - 18,
        x + 5, y + 10,
        x,     y + 26,
        x - 5, y + 10,
    };
    renderer_color(0.85f, 0.88f, 0.92f, 1.0f);
    renderer_polygon(body, 5);

    /* cabine */
    renderer_color(0.3f, 0.75f, 1.0f, 1.0f);
    renderer_rect(x - 2.5f, y + 4, 5, 9);
}

static void meteor_shape(const Meteor *m, float shrink, float *xy)
{
    float step = 2.0f * PI / METEOR_SIDES;
    float rot  = m->angle * PI / 180.0f;
    for (int k = 0; k < METEOR_SIDES; k++) {
        float a = rot + k * step;
        float r = m->radius * m->shape[k] - shrink;
        xy[2 * k]     = m->x + cosf(a) * r;
        xy[2 * k + 1] = m->y + sinf(a) * r;
    }
}

static void draw_meteor(const Meteor *m)
{
    float xy[METEOR_SIDES * 2];

    /* contorno: poligono escuro inteiro, depois o claro por cima */
    meteor_shape(m, 0.0f, xy);
    renderer_color(0.3f, 0.2f, 0.12f, 1.0f);
    renderer_polygon(xy, METEOR_SIDES);

    meteor_shape(m, 2.5f, xy);
    renderer_color(0.5f, 0.35f, 0.25f, 1.0f);
    renderer_polygon(xy, METEOR_SIDES);
}

static void draw_progress_bar(const Game *g)
{
    float w = WIDTH - 40.0f;
    float p = g->elapsed / WIN_TIME;

    renderer_color(0.2f, 0.2f, 0.25f, 1.0f);
    renderer_rect(20, HEIGHT - 24, w, 10);
    renderer_color(0.2f, 0.8f, 0.35f, 1.0f);
    renderer_rect(20, HEIGHT - 24, w * p, 10);
}

static void draw_hud(const Game *g, const Sprites *sp)
{
    const float scale = 0.5f;
    float y = HEIGHT - 36 - sp->points.h * scale;

    sprites_draw(&sp->points, 20, y, scale);
    sprites_draw_number(sp, g->points,
                        20 + sp->points.w * scale + 4, y, scale);

    char buf[32];
    snprintf(buf, sizeof buf, "%.1f / %.0f", g->elapsed, WIN_TIME);
    float s = 3.0f;
    text_draw(buf, WIDTH - 20 - text_width(buf, s), y + 7, s,
              1.0f, 1.0f, 1.0f);
}

static void draw_overlay(float r, float g, float b, float a)
{
    renderer_color(r, g, b, a);
    renderer_rect(0, 0, WIDTH, HEIGHT);
}

static void draw_menu(const Game *g, const Sprites *sp)
{
    text_draw_centered("METEOR DODGE", 440, 7, YELLOW_R, YELLOW_G, YELLOW_B);
    text_draw_centered("DESVIE DOS METEOROS POR 60 SEGUNDOS", 390, 2.5f,
                       1.0f, 1.0f, 1.0f);

    int   hover = inside_start(sp, g->mouse_x, g->mouse_y);
    float pulse = hover ? 1.1f : 1.0f;
    float w     = sp->start.w * START_SCALE * pulse;
    float h     = sp->start.h * START_SCALE * pulse;
    renderer_color(1, 1, 1, 1);
    renderer_sprite(&sp->start, (WIDTH - w) / 2.0f,
                    START_Y - (h - sp->start.h * START_SCALE) / 2.0f, w, h);

    text_draw_centered("ENTER OU CLIQUE PARA COMECAR", 200, 2.5f,
                       0.85f, 0.85f, 0.85f);
    text_draw_centered("SETAS / WASD: MOVER   ESC: SAIR", 170, 2.5f,
                       0.85f, 0.85f, 0.85f);
}

static void draw_end_screen(const Game *g, const Sprites *sp)
{
    if (g->state == GAME_OVER) {
        draw_overlay(0.6f, 0.0f, 0.0f, 0.45f);
        sprites_draw_centered(&sp->game_over, 330, 1.0f);
    } else {
        draw_overlay(0.0f, 0.5f, 0.15f, 0.45f);
        text_draw_centered("VITORIA!", 340, 9, YELLOW_R, YELLOW_G, YELLOW_B);
    }

    sprites_draw_points_centered(sp, g->points, 250, 0.75f);
    text_draw_centered("R: JOGAR DE NOVO   M: MENU   ESC: SAIR", 200, 2.5f,
                       1.0f, 1.0f, 1.0f);
}

void game_render(const Game *g, const Sprites *sp)
{
    draw_stars(g);

    if (g->state == MENU) {
        draw_player(&g->player, 1);
        draw_menu(g, sp);
        return;
    }

    for (int i = 0; i < MAX_METEORS; i++)
        if (g->meteors[i].active)
            draw_meteor(&g->meteors[i]);
    draw_player(&g->player, g->state == PLAYING);
    draw_progress_bar(g);

    if (g->state == PLAYING)
        draw_hud(g, sp);
    else
        draw_end_screen(g, sp);
}
