#ifndef APP_H
#define APP_H

// TODO: Add this to drift
#define Abs(x) ((x) > 0 ? (x) : -(x))

typedef enum mode
{
    MENU_MODE,
    GAME_MODE,
    PAUSE_MODE,
    EDITOR_MODE
} mode;

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
    b32 active;
    texture object_texture;
    b32 initialized;
} object;

typedef struct app_state
{
    renderer renderer;
    f32 dt;
    f32 time;
    mode mode;

    entity player;
    u32 platform_count;
    object platforms[50];
    u32 coin_count;
    object coins[100]; 
} app_state;

#endif
