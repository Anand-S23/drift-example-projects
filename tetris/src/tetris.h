#ifndef TETRIS_H
#define TETRIS_H

#define WIDTH      10
#define HEIGHT     22
#define VIS_HEIGHT 20
#define CELL_SIZE  30

typedef enum tetromino_type
{
    TETROMINO_i,
    TETROMINO_o,
    TETROMINO_t,
    TETROMINO_j,
    TETROMINO_l,
    TETROMINO_s,
    TETROMINO_z
} tetromino_type;

const u8 TI[] = {
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0,
    0, 0, 0, 0
};

const u8 TO[] = {
    2, 2,
    2, 2
};

const u8 TT[] = {
    0, 0, 0,
    3, 3, 3,
    0, 3, 0
};

const u8 TJ[] = {
    4, 0, 0,
    4, 4, 4,
    0, 0, 0
};

const u8 TL[] = {
    0, 0, 5,
    5, 5, 5,
    0, 0, 0
};

const u8 TS[] = {
    0, 6, 6,
    6, 6, 0,
    0, 0, 0
};

const u8 TZ[] = {
    7, 7, 0,
    0, 7, 7,
    0, 0, 0
};

typedef struct tetromino
{
    const u8 *data;
    int size;
} tetromino;

tetromino tetrominos[] = {
    { TI, 4 },
    { TO, 2 },
    { TT, 3 },
    { TJ, 3 },
    { TL, 3 },
    { TS, 3 },
    { TZ, 3 },
};

typedef struct piece
{
    tetromino_type type;
    int row_offset;
    int col_offset;
    int rotation;
} piece;

typedef struct app_state
{
    renderer renderer;
    int delta_t;

    u8 board[WIDTH * HEIGHT];
    piece current_piece;
    b32 game_over; 
    int line_count;
    int time_since_last_drop;
} app_state;

#endif
