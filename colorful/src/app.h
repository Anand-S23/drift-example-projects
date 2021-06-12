#ifndef APP_H
#define APP_H

// Orange, red orange, purple, violet, blue, turquiose, yellow

enum
{
    COLOR_orange,
    COLOR_red,
    COLOR_purple,
    COLOR_violet,
    COLOR_blue,
    COLOR_turquiose,
    COLOR_yellow
};

typedef struct square
{
    v2 pos;
    v2 size;
    u8 c_index;
    v4 color;
} square;

typedef struct app_state
{
    renderer renderer;
    f32 delta_t;

    v4 colors[7];
    square squares[7];
} app_state;

#endif
