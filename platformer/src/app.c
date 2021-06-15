#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <drift.h>
#include "app.h"

global app_state *state;

INIT_APP
{
    Assert(sizeof(app_state) <= platform->storage_size);
    state = (app_state *)platform->storage;

    srand(time(0));
    InitRenderer(&state->renderer);

    platform->initialized = 1;
}

UPDATE_APP
{
    state->delta_t = (platform->current_time - platform->last_time) / 100.f;

    local_persist entity player = {0};
    if (!player.initialized)
    {
        player.dimension = v2(32, 64);
        player.acc = v2(0.75f, 9.8f);
        player.max_speed = 10.f;
        player.jump_speed = 30.f;
        player.friction = 1.f;
        player.initialized = 1;
    }

    rigid_body test_platform = (rigid_body){
        .pos = v2(200.f, 400.f),
        .size = v2(150.f, 20.f)
    };

    // Update

    if (!player.is_jumping)
    {
        // Player movement left and right
        if (platform->keys[KEY_right].down)
        {
            player.vel.x += (player.max_speed * player.acc.x * state->delta_t);
            player.vel.x = Min(player.vel.x, player.max_speed);
        }
        else if (platform->keys[KEY_left].down)
        {
            player.vel.x -= (player.max_speed * player.acc.x * state->delta_t);
            player.vel.x = Max(player.vel.x, -player.max_speed);
        }
        else
        {
            // Slow to a stop with friction
            if (player.vel.x > 0)
            {
                player.vel.x -= (player.friction * player.max_speed * state->delta_t);
                if (player.vel.x < 0)
                {
                    player.vel.x = 0;
                }
            }
            else if (player.vel.x < 0)
            {
                player.vel.x += (player.friction * player.max_speed * state->delta_t);
                if (player.vel.x > 0)
                {
                    player.vel.x = 0;
                }
            }
        }
    }

    // Player jump
    if (player.is_grounded && platform->keys[KEY_up].begin_down)
    {
        player.is_jumping = 1;
        player.is_grounded = 0;
        player.vel.y = -player.jump_speed;
    }

    // Apply gravity
    if (!player.is_grounded)
    {
        player.vel.y += (player.acc.y * state->delta_t);
    }
    else
    {
        player.vel.y = 0;
    }

    // Collision detection
    if (player.pos.x < test_platform.pos.x + test_platform.size.width &&
        player.pos.x + player.dimension.width > test_platform.pos.x &&
        player.pos.y < test_platform.pos.y + test_platform.size.height &&
        player.pos.y + player.dimension.height > test_platform.pos.y)
    {
        if (player.vel.y > 0)
        {
            player.pos.y = test_platform.pos.y - player.dimension.height;
            player.vel = v2(0, 0);
            player.is_grounded = 1;
            player.is_jumping = 0;
        }
        else if (player.vel.x > 0)
        {
            player.pos.x = test_platform.pos.x - player.dimension.width - 1;
            player.vel.x = 0;
        }
        else if (player.vel.x < 0)
        {
            player.pos.x = test_platform.pos.x + test_platform.size.width + 1;
            player.vel.x = 0;
        }
    }
        

    DriftLog("(%lf, %lf) : %lf - %lf", player.vel.x, player.vel.y,
             (player.jump_speed * state->delta_t));

    if (player.is_grounded && platform->keys[KEY_down].down)
    {
        player.dimension.height = 32;
    }
    else
    {
        player.dimension.height = 64;
    }

    player.pos = V2Add(player.pos, player.vel);

    if (player.pos.x < 0)
    {
        player.pos.x = 0;
    }
    else if (player.pos.x + player.dimension.width > platform->window_width)
    {
        player.pos.x = platform->window_width - player.dimension.width;
    }

    if (player.pos.y < 0)
    {
        player.pos.y = 0;
    }
    else if (player.pos.y + player.dimension.height > platform->window_height)
    {
        player.pos.y = platform->window_height - player.dimension.height;
        player.is_grounded = 1;
        player.is_jumping = 0;
    }

    // Render
    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    RenderRect(&state->renderer, player.pos, player.dimension, v4(0, 1, 1, 1));
    RenderRect(&state->renderer, test_platform.pos, test_platform.size, v4(1, 1, 0, 1));

    SubmitRenderer(&state->renderer);
    platform->SwapBuffers();
}

DRIFT_MAIN
{
    drift_application app = {
        .name = "Platformer",
        .window_width = 640,
        .window_height = 480
    };

    platform = platform_;
    return app;
}
