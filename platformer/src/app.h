#ifndef APP_H
#define APP_H

#define PLAYER_ACC 0.1f
#define PLAYER_FRICTION 0.1f
#define GRAVITY 0.5f

typedef struct entity
{
    v2 pos;
    v2 vel;
    v2 acc;
    v2 dimension;
    texture entity_texture;
    b32 grounded;
    b32 initialized;
} entity;

typedef struct app_state
{
    renderer renderer;
    f32 delta_t;
} app_state;

#endif
