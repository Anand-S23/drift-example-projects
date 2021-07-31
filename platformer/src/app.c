#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <drift.h>
#include "app.h"

global app_state *app;

INIT_APP
{
    Assert(sizeof(app_state) <= platform->storage_size);
    app = (app_state *)platform->storage;

    srand(time(0));
    InitRenderer(&app->renderer);

    platform->initialized = 1;
}

UPDATE_APP
{
    app->dt = (platform->current_time - platform->last_time) / 100.f;
    app.time += dt;

    // Init entities // 

    local_persist entity player = {0};
    local_persist object test_platform = {0};
    local_persist object coin = {0};

    if (!player.initialized)
    {
        player.size = v2(32, 64);
        player.acc = v2(0.75f, 9.8f);
        player.max_speed = 10.f;
        player.jump_speed = 30.f;
        player.friction = 1.f;
        player.initialized = 1;
    }

    if (!test_platform.initialized)
    {
        test_platform.pos = v2(200.f, 400.f);
        test_platform.size = v2(150.f, 20.f);
        test_platform.intialized = 1;
    }

    if (!coin.initialized)
    {
        coin.pos = v2(250.f, 350.f);
        coin.size = v2(25.f, 25.f);
        coin.initialized = 1;
    }

    // Update // 

    if (!player.is_jumping)
    {
        // Player movement left and right
        if (platform->keys[KEY_right].down)
        {
            player.vel.x += (player.max_speed * player.acc.x * app->dt);
            player.vel.x = Min(player.vel.x, player.max_speed);
        }
        else if (platform->keys[KEY_left].down)
        {
            player.vel.x -= (player.max_speed * player.acc.x * app->dt);
            player.vel.x = Max(player.vel.x, -player.max_speed);
        }
        else
        {
            // Slow to a stop with friction
            if (player.vel.x > 0)
            {
                player.vel.x -= (player.friction * player.max_speed * app->dt);
                if (player.vel.x < 0)
                {
                    player.vel.x = 0;
                }
            }
            else if (player.vel.x < 0)
            {
                player.vel.x += (player.friction * player.max_speed * app->dt);
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
        player.vel.y += (player.acc.y * app->dt);
    }
    else
    {
        player.vel.y = 0;
    }

    // Collision detection
    if (player.pos.x < test_platform.pos.x + test_platform.size.width &&
        player.pos.x + player.size.width > test_platform.pos.x &&
        player.pos.y <= test_platform.pos.y + test_platform.size.height &&
        player.pos.y + player.size.height >= test_platform.pos.y)
    {
        if (player.vel.y > 0)
        {
            player.pos.y = test_platform.pos.y - player.size.height;
            player.vel.y = 0;
            player.is_grounded = 1;
            player.is_jumping = 0;
            player.is_colliding = 1;
        }

        if (!player.is_colliding)
        {
            /*
            if (player.vel.x > 0)
            {
                player.pos.x =
                    test_platform.pos.x - player.size.width;
                player.vel.x = 0;
            }
            else if (player.vel.x < 0)
            {
                player.pos.x =
                    test_platform.pos.x + test_platform.size.width;
                player.vel.x = 0;
            }
            */
            int far = test_platform.pos.x + test_platform.size.width;
            int near = test_platform.pos.x;
            int near_offset = near - player.size.width;
            int far_dx = Abs((player.pos.x - far));
            int near_dx = Abs((player.pos.x - near));

            player.pos.x = far_dx > near_dx ? far : near_offset;
            player.vel.x = 0;
        }
    }
    else
    {
        if (player.is_colliding)
        {
            player.is_colliding = 0;
            player.is_grounded = 0;
        }
    }

    DriftLog("(%lf, %lf) : %lf - %lf", player.vel.x, player.vel.y,
             (player.jump_speed * app->dt));

    if (player.is_grounded && platform->keys[KEY_down].down)
    {
        player.size.height = 32;
    }
    else
    {
        player.size.height = 64;
    }

    player.pos = V2Add(player.pos, player.vel);

    if (player.pos.x < 0)
    {
        player.pos.x = 0;
    }
    else if (player.pos.x + player.size.width > platform->window_width)
    {
        player.pos.x = platform->window_width - player.size.width;
    }

    if (player.pos.y < 0)
    {
        player.pos.y = 0;
    }
    else if (player.pos.y + player.size.height > platform->window_height)
    {
        player.pos.y = platform->window_height - player.size.height;
        player.is_grounded = 1;
        player.is_jumping = 0;
    }

    // Update coin
    coin.pos.y += sin(app->time);
    
    // Render // 

    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&app->renderer, platform->window_width, platform->window_height);

    RenderRect(&app->renderer, player.pos, player.size, v4(0, 1, 1, 1));
    RenderRect(&app->renderer, test_platform.pos, test_platform.size, v4(1, 1, 0, 1));

    SubmitRenderer(&app->renderer);
    platform->SwapBuffers();
}

DRIFT_MAIN
{
    // TODO: dynamically set window dimension in drift
    drift_application app = {
        .name = "Platformer",
        .window_width = 640,
        .window_height = 480
    };

    platform = platform_;
    return app;
}
