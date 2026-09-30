#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "sprites.h"


typedef enum { MENU, PLAYING, GAME_OVER, VICTORY } GameState;

typedef struct {
    float x, y;
} Player;

typedef struct {
    float x, y;                
    float vx, speed;           
    float radius;
    float angle, spin;         
    float shape[METEOR_SIDES]; 
    int   active;
} Meteor;

typedef struct {
    float x, y, speed;
} Star;


typedef struct {
    int left, right, up, down;
} Input;

typedef enum { KEY_START, KEY_RESTART, KEY_MENU } GameKey;

typedef struct {
    GameState state;
    Player    player;
    Meteor    meteors[MAX_METEORS];
    Star      stars[NUM_STARS];
    float     elapsed;
    float     spawn_timer;
    int       points;
    float     mouse_x, mouse_y;
} Game;

void game_init(Game *g);
void game_update(Game *g, const Input *in, float dt);
void game_render(const Game *g, const Sprites *sp);

void game_key(Game *g, GameKey key);
void game_click(Game *g, float x, float y, const Sprites *sp);

#endif
