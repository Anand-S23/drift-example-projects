#ifndef APP_H
#define APP_H

// TODO: Add this to drift
#define Abs((x)) ((x) > 0 ? (x) : -(x))

typedef struct entity
{
    v2 pos;
    v2 vel;
    v2 acc;
    v2 size;
    f32 max_speed;
    f32 friction;
    f32 jump_speed;
    b32 is_grounded;
    b32 is_jumping;
    b32 is_colliding;
    texture entity_texture;
    b32 initialized;
} entity;

typedef struct object
{
    v2 pos;
    v2 size;
    texture object_texture;
    b32 initialized;
} object;

typedef struct app_state
{
    renderer renderer;
    f32 dt;
    f32 time;
} app_state;

#endif
