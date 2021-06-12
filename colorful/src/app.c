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

    state->colors[0] = v4(1.f, 0.65f, 0.f, 1.f);
    state->colors[1] = v4(1.f, 0.f, 0.f, 1.f);
    state->colors[2] = v4(0.58f, 0.f, 0.83f, 1.f);
    state->colors[3] = v4(0.9f, 0.9f, 0.98f, 1.f);
    state->colors[4] = v4(0.f, 0.59f, 1.f, 1.f);
    state->colors[5] = v4(0.f, 0.5f, 0.5f, 1.f);
    state->colors[6] = v4(1.f, 1.f, 0.f, 1.f);

    int width = 25;
    int height = 300;
    int padding = 10;

    int current_pos = padding;
    for (int i = 0; i < 7; ++i)
    {
        square *current = &state->squares[i];
        {
            current->pos = v2(current_pos, 80);
            current->size = v2(width, height);
            current->c_index = i;
            current->color = state->colors[i];
        }

        current_pos += width + 2 * padding;
    }

    platform->initialized = 1;
}

UPDATE_APP
{
    state->delta_t = (platform->current_time - platform->last_time) / 100.f;

    // Update
    local_persist int count = 0;
    local_persist f32 slider_value = 0.f;

    if (count++ == (int)(5 * slider_value + 1))
    {
        for (int i = 0; i < 7; ++i)
        {
            square *current = &state->squares[i];
            current->c_index += 1;
            if (current->c_index >= 7)
            {
                current->c_index = 0;
            }

            current->color = state->colors[current->c_index];
        }

        count = 0;
    }

    // Render
    ClearScreen(v4(0.2f, 0.2f, 0.2f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    for (int i = 0; i < 7; ++i)
    {
        square current = state->squares[i];
        RenderRect(&state->renderer, current.pos, current.size, current.color);
    }

    local_persist ui app_ui = {0};
    UIBeginFrame(&app_ui, &state->renderer);
    {
        slider_value = UISlider(&app_ui, UIIDGen(), v2(400, 250),
                                v2(150, 50), slider_value);
    }
    UIEndFrame(&app_ui);

    SubmitRenderer(&state->renderer);
    platform->SwapBuffers();

}

DRIFT_MAIN
{
    drift_application app = {
        .name = "colorful",
        .window_width = 640,
        .window_height = 480
    };

    platform = platform_;
    return app;
}

