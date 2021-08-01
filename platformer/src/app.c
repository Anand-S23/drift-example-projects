#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <drift.h>
#include "app.h"

global app_state *app;

internal b32 PositionBound(v2 pos, v4 rect)
{
    return (pos.x > rect.x && pos.x < rect.x + rect.width &&
            pos.y > rect.y && pos.y < rect.y + rect.height);
}

internal b32 EntityObjectCollision(entity *e, object *o)
{
    return (e->pos.x < o->pos.x + o->size.width &&
            e->pos.x + e->size.width > o->pos.x &&
            e->pos.y <= o->pos.y + o->size.height &&
            e->pos.y + e->size.height >= o->pos.y);
}

internal void UpdateGame(void)
{
    // Update // 

    // Player movement left and right
    if (!app->player.is_jumping)
    {
        if (platform->keys[KEY_right].down)
        {
            app->player.vel.x += (app->player.max_speed * app->player.acc.x * app->dt);
            app->player.vel.x = Min(app->player.vel.x, app->player.max_speed);
        }
        else if (platform->keys[KEY_left].down)
        {
            app->player.vel.x -= (app->player.max_speed * app->player.acc.x * app->dt);
            app->player.vel.x = Max(app->player.vel.x, -app->player.max_speed);
        }
        else
        {
            // Slow to a stop with friction
            if (app->player.vel.x > 0)
            {
                app->player.vel.x -= (app->player.friction * app->player.max_speed * app->dt);
                if (app->player.vel.x < 0)
                {
                    app->player.vel.x = 0;
                }
            }
            else if (app->player.vel.x < 0)
            {
                app->player.vel.x += (app->player.friction * app->player.max_speed * app->dt);
                if (app->player.vel.x > 0)
                {
                    app->player.vel.x = 0;
                }
            }
        }
    }

    // Player jump
    // TODO: Jump timer
    if (app->player.is_grounded && platform->keys[KEY_up].begin_down)
    {
        app->player.is_jumping = 1;
        app->player.is_grounded = 0;
        app->player.vel.y = -app->player.jump_speed;
    }

    // Collision detection
    for (int i = 0; i < app->platform_count; ++i)
    {
        object current_platform = app->platforms[i];
        if (EntityObjectCollision(&app->player, &current_platform))
        {
            if (app->player.vel.y > 0)
            {
                app->player.pos.y = current_platform.pos.y - app->player.size.height;
                app->player.vel.y = 0;
                app->player.is_grounded = 1;
                app->player.is_jumping = 0;
                app->player.is_colliding = 1;
            }

            if (!app->player.is_colliding)
            {
                // Stop player from walking into wall 
                int far = current_platform.pos.x + current_platform.size.width;
                int near = current_platform.pos.x;
                int near_offset = near - app->player.size.width;
                int far_dx = Abs((app->player.pos.x - far));
                int near_dx = Abs((app->player.pos.x - near));

                app->player.pos.x = far_dx < near_dx ? far : near_offset;
                app->player.vel.x *= 0.5f;
            }
        }
        else
        {
            if (app->player.is_colliding)
            {
                app->player.is_colliding = 0;
                app->player.is_grounded = 0;
            }
        }
    }

    // Update player position
    app->player.pos = V2Add(app->player.pos, app->player.vel);

    if (!app->player.is_grounded)
    {
        app->player.vel.y += (app->player.acc.y * app->dt);
    }
    else
    {
        app->player.vel.y = 0;
    }

    // Keeps player in bound
    if (app->player.pos.x < 0)
    {
        app->player.pos.x = 0;
    }
    else if (app->player.pos.x + app->player.size.width > platform->window_width)
    {
        app->player.pos.x = platform->window_width - app->player.size.width;
    }

    if (app->player.pos.y < 0)
    {
        app->player.pos.y = 0;
    }
    else if (app->player.pos.y + app->player.size.height > platform->window_height)
    {
        app->player.pos.y = platform->window_height - app->player.size.height;
        app->player.is_grounded = 1;
        app->player.is_jumping = 0;
    }

    // Update coin pos
    for (int i = 0; i < app->coin_count; ++i)
    {
        object *c = &app->coins[i];
        c->pos.y += 1.5f * sinf(app->time);

        // Player coin collision
        if (EntityObjectCollision(&app->player, c))
        {
            c->active = 0;
        }
    }
}

internal void RenderGame(void)
{
    // Render // 

    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&app->renderer, platform->window_width, platform->window_height);

    RenderRect(&app->renderer, app->player.pos, app->player.size, v4(0, 1, 1, 1));

    for (int i = 0; i < app->platform_count; ++i)
    {
        object *p = &app->platforms[i];
        RenderRect(&app->renderer, p->pos, p->size, v4(1, 1, 0, 1));
    }

    for (int i = 0; i < app->coin_count; ++i)
    {
        object *c = &app->coins[i];
        if (c->active)
        {
            RenderRect(&app->renderer, c->pos, c->size, v4(1, 1, 0, 1));
        }
    }

    SubmitRenderer(&app->renderer);
}

internal void UpdateEditor(void)
{
    v2 mouse_pos = v2(platform->mouse_x, platform->mouse_y); 
    v4 player_rect = v4(app->player.pos.x, app->player.pos.y,
                        app->player.size.width, app->player.size.height);

    if (PositionBound(mouse_pos, player_rect) && platform->left_mouse_down)
    {
        v2 mouse_offset = V2Subtract(app->player.pos, mouse_pos);
        app->player.pos = V2Subtract(mouse_pos, mouse_offset);
    }
    else if (platform->left_mouse_down)
    {
        object new_platform = (object) {
            .pos = mouse_pos,
            .size = v2(150.f, 25.f),
            .active = 1,
            .initialized = 1
        };

        app->platforms[app->platform_count++] = new_platform;
    }
    else if (platform->right_mouse_down)
    {
        object new_coin = (object) {
            .pos = mouse_pos,
            .size = v2(25.f, 25.f),
            .active = 1,
            .initialized = 1
        };

        app->coins[app->coin_count++] = new_coin;
    }
}

internal void RenderEditor(void)
{
    // Render // 

    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&app->renderer, platform->window_width, platform->window_height);

    RenderRect(&app->renderer, app->player.pos, app->player.size, v4(0, 1, 1, 1));

    for (int i = 0; i < app->platform_count; ++i)
    {
        object *p = &app->platforms[i];
        RenderRect(&app->renderer, p->pos, p->size, v4(1, 1, 0, 1));
    }

    for (int i = 0; i < app->coin_count; ++i)
    {
        object *c = &app->coins[i];
        if (c->active)
        {
            RenderRect(&app->renderer, c->pos, c->size, v4(1, 1, 0, 1));
        }
    }

    // TODO: Add UI

    SubmitRenderer(&app->renderer);
}

INIT_APP
{
    Assert(sizeof(app_state) <= platform->storage_size);
    app = (app_state *)platform->storage;
    app->mode = GAME_MODE;

    InitRenderer(&app->renderer);

    // Init entities 
    if (!app->player.initialized)
    {
        app->player.size = v2(32, 64);
        app->player.acc = v2(0.75f, 9.8f);
        app->player.max_speed = 10.f;
        app->player.jump_speed = 30.f;
        app->player.friction = 1.f;
        app->player.initialized = 1;
    }

    object *p = &app->platforms[app->platform_count++];
    {
        p->pos = v2(200.f, 400.f);
        p->size = v2(150.f, 20.f);
        p->active = 1;
        p->initialized = 1;
    }

    object *c = &app->coins[app->coin_count++];
    {
        c->pos = v2(250.f, 350.f);
        c->size = v2(25.f, 25.f);
        c->active = 1;
        c->initialized = 1;
    }

    platform->initialized = 1;
}

UPDATE_APP
{
    app->dt = (platform->current_time - platform->last_time) / 100.f;
    app->time += app->dt;

    // Toggle game / editor mode
    if (platform->keys[KEY_esc].begin_down)
    {
        if (app->mode != GAME_MODE)
        {
            app->mode = GAME_MODE;
        }
        else
        {
            app->mode = EDITOR_MODE; 
        }
    }

    switch (app->mode)
    {
        case MENU_MODE:
        {
        } break;

        case GAME_MODE:
        {
            UpdateGame();
            RenderGame();
        } break;

        case PAUSE_MODE:
        {
        } break;

        case EDITOR_MODE:
        {
            UpdateEditor();
            UpdateGame();
            RenderEditor();
        } break;
    }

    platform->SwapBuffers();
}

DRIFT_MAIN
{
    // TODO: dynamically set window dimension in drift
    drift_application app = {
        .name = "Platformer",
        .window_width = 1280,
        .window_height = 720
    };

    platform = platform_;
    return app;
}
