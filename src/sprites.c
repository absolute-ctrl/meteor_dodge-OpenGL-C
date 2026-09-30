#include "sprites.h"
#include "config.h"

#include <stdio.h>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

static int find_asset(const char *name, char *out, size_t size)
{
    const char *prefixes[] = {"assets/", "../assets/", ASSETS_DIR "/"};
    for (size_t i = 0; i < sizeof prefixes / sizeof prefixes[0]; i++) {
        snprintf(out, size, "%s%s", prefixes[i], name);
        FILE *f = fopen(out, "rb");
        if (f) {
            fclose(f);
            return 1;
        }
    }
    fprintf(stderr, "Arquivo nao encontrado: %s\n", name);
    return 0;
}

static int load(Texture *tex, const char *name)
{
    char path[512];
    return find_asset(name, path, sizeof path) && texture_load(tex, path);
}

int sprites_load(Sprites *sp)
{
    int ok = 1;
    ok &= load(&sp->start,     "sprites/start.png");
    ok &= load(&sp->points,    "sprites/points.png");
    ok &= load(&sp->game_over, "sprites/game_over.png");

    for (int i = 0; i < 10; i++) {
        char name[64];
        snprintf(name, sizeof name, "sprites/digits/%d.png", i);
        ok &= load(&sp->digits[i], name);
    }
    return ok;
}

void sprites_free(Sprites *sp)
{
    texture_free(&sp->start);
    texture_free(&sp->points);
    texture_free(&sp->game_over);
    for (int i = 0; i < 10; i++)
        texture_free(&sp->digits[i]);
}

void sprites_draw(const Texture *tex, float x, float y, float scale)
{
    renderer_color(1, 1, 1, 1);
    renderer_sprite(tex, x, y, tex->w * scale, tex->h * scale);
}

void sprites_draw_centered(const Texture *tex, float y, float scale)
{
    sprites_draw(tex, (WIDTH - tex->w * scale) / 2.0f, y, scale);
}

static int split_digits(int value, int out[12])
{
    int n = 0;
    if (value < 0)
        value = 0;
    do {
        out[n++] = value % 10;
        value /= 10;
    } while (value > 0 && n < 12);
    return n;
}

float sprites_number_width(const Sprites *sp, int value, float scale)
{
    int d[12], n = split_digits(value, d);
    float w = 0;
    for (int i = 0; i < n; i++)
        w += sp->digits[d[i]].w * scale;
    return w;
}

void sprites_draw_number(const Sprites *sp, int value,
                         float x, float y, float scale)
{
    int d[12], n = split_digits(value, d);
    for (int i = n - 1; i >= 0; i--) {
        const Texture *t = &sp->digits[d[i]];
        sprites_draw(t, x, y, scale);
        x += t->w * scale;
    }
}

void sprites_draw_points_centered(const Sprites *sp, int value,
                                  float y, float scale)
{
    float gap = 8 * scale;
    float w = sp->points.w * scale + gap + sprites_number_width(sp, value, scale);
    float x = (WIDTH - w) / 2.0f;

    sprites_draw(&sp->points, x, y, scale);
    sprites_draw_number(sp, value, x + sp->points.w * scale + gap, y, scale);
}
