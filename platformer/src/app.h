#ifndef APP_H
#define APP_H

typedef struct entity
{
    v2 pos;
    v2 vel;
    v2 acc;
    v2 dimension;
    f32 max_speed;
    f32 friction;
    f32 jump_speed;
    b32 is_grounded;
    b32 is_jumping;
    b32 is_colliding;
    texture entity_texture;
    b32 initialized;
} entity;

typedef struct rigid_body
{
    v2 pos;
    v2 size;
} rigid_body;

#define MAX_PLATFORMS 5

typedef struct app_state
{
    renderer renderer;
    f32 delta_t;
    rigid_body platforms[MAX_PLATFORMS];
} app_state;

#endif
