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
    state->delta_t = platform->current_time - platform->last_time;

    local_persist entity player = {0};
    if (!player.initialized)
    {
        player.dimension = v2(32, 64);
        player.initialized = 1;
    }

    // Update
    if (platform->keys[KEY_right].down)
    {
        player.acc.x = PLAYER_ACC;
    }
    else if (platform->keys[KEY_left].down)
    {
        player.acc.x = -PLAYER_ACC;
    }
    else
    {
        player.acc.x = 0;
    }

    if (!player.grounded)
    {
        player.acc.y = GRAVITY;
    }
    else
    {
        player.acc.y = 0;
    }

    _DriftLog("%d, %d", player.acc.x, player.acc.y);

    if (player.grounded && platform->keys[KEY_down].down)
    {
        player.dimension.height = 32;
    }
    else
    {
        player.dimension.height = 64;
    }

    player.acc.x += player.vel.x * 0.1f;
    player.vel = V2Scalar(V2Add(player.vel, player.acc), 2);
    player.pos = V2Add(player.pos, V2Add(player.vel, V2Scalar(player.acc, 0.5)));

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
        player.grounded = 1;
    }

    // Render
    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    RenderRect(&state->renderer, player.pos, player.dimension, v4(0, 1, 1, 1));

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
