#ifndef SPRITES_H
#define SPRITES_H

#include "renderer.h"


typedef struct {
    Texture start;       
    Texture points;      
    Texture game_over;   
    Texture digits[10];  
} Sprites;

int  sprites_load(Sprites *sp);
void sprites_free(Sprites *sp);

void sprites_draw(const Texture *tex, float x, float y, float scale);

void sprites_draw_centered(const Texture *tex, float y, float scale);

float sprites_number_width(const Sprites *sp, int value, float scale);
void  sprites_draw_number(const Sprites *sp, int value,
                          float x, float y, float scale);

void sprites_draw_points_centered(const Sprites *sp, int value,
                                  float y, float scale);

#endif
