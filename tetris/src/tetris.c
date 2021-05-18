#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <drift.h>
#include <drift_renderer.c>

#include "tetris.h"

global app_state *state;

internal u8 GetTetrominoData(tetromino *tetromino, int row, int col, int rotation)
{
    int size = tetromino->size;
    u8 val = 0;

    switch (rotation)
    {
        case 0:
        {
            val = tetromino->data[row * size + col];
        } break;

        case 1:
        {
            val = tetromino->data[(size - col - 1) * size + row];
        } break;

        case 2:
        {
            val = tetromino->data[(size - row - 1) * size + (size - col - 1)];
        } break;

        case 3:
        {
            val = tetromino->data[col * size + (size - row - 1)];
        } break;

        default: break;
    }

    return val;
}

internal b32 CheckPieceValid(piece *piece, const u8 *board, int width, int height)
{
    tetromino *tetro = &tetrominos[piece->type];

    for (int row = 0; row < tetro->size; ++row)
    {
        for (int col = 0; col < tetro->size; ++col)
        {
            u8 val = GetTetrominoData(tetro, row, col, piece->rotation);
            if (val)
            {
                int board_row = piece->row_offset + row;
                int board_col = piece->col_offset + col;

                if (board_row < 0)
                {
                    return 0;
                }

                if (board_row >= height)
                {
                    return 0;
                }

                if (board_col < 0)
                {
                    return 0;
                }

                if (board_col >= width)
                {
                    return 0;
                }

                if (board[board_row * width + board_col])
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

internal b32 GetUniformLine(u8 *board, int row, int val, int width)
{
    for (int col = 0; col < width; ++col)
    {
        if ((val && !board[row * width + col]) ||
            (!val && board[row * width + col]))
        {
            return 0;
        }
    }

    return 1;
}

internal void MergePieceToBoard(app_state *game_state)
{
    tetromino *tetro = &tetrominos[game_state->current_piece.type];

    for (int row = 0; row < tetro->size; ++row)
    {
        for (int col = 0; col < tetro->size; ++col)
        {
            u8 val = GetTetrominoData(tetro, row, col,
                                      game_state->current_piece.rotation);
            if (val)
            {
                int board_row = game_state->current_piece.row_offset + row;
                int board_col = game_state->current_piece.col_offset + col;
                game_state->board[board_row * WIDTH + board_col] = val;
                state->game_over = (board_row < 2) ? 1 : 0;
            }
        }
    }
}
            
internal void CheckClearLine(u8 *board)
{
    b32 cleared_lines[HEIGHT] = {0};
    int cleared_count = 0;

    for (int row = HEIGHT - 1; row >= 0; --row)
    {
        if (GetUniformLine(board, row, 1, WIDTH))
        {
            cleared_lines[row] = 1;
            ++cleared_count;
        }
    }

    if (cleared_count)
    {
        int next_line = HEIGHT - 1;
        for (int row = HEIGHT - 1; row >= 0; --row)
        {
            while (cleared_lines[next_line] && next_line >= 0)
            {
                --next_line;
            }

            if (next_line < 0)
            {
                memset(&board[row * WIDTH], 0, WIDTH);
            }
            else
            {
                if (row != next_line)
                {
                    memcpy(&board[row * WIDTH],
                           &board[next_line * WIDTH], WIDTH);
                }

                --next_line;
            }
        }
    }
}

internal void SpawnNewPiece(app_state *game_state)
{
    game_state->current_piece.type = (tetromino_type)(rand() % 7);
    game_state->current_piece.row_offset = 0;
    game_state->current_piece.col_offset = WIDTH / 2;
    game_state->current_piece.rotation = 0;
}

internal b32 SoftDropPiece(app_state *game_state)
{
    ++game_state->current_piece.row_offset;

    if (!CheckPieceValid(&game_state->current_piece,
                         game_state->board, WIDTH, HEIGHT))
    {
        --game_state->current_piece.row_offset;
        MergePieceToBoard(game_state);
        CheckClearLine(game_state->board);
        SpawnNewPiece(game_state);
        return 0;
    }

    return 1;
}

internal v4 GetColor(int val, b32 game_over)
{
    v4 color;

    if (game_over)
    {
        color = v4(0.5f, 0.5f, 0.5f, 1);
    }
    else
    {
        switch (val)
        {
            case 1:
            {
                color = v4(0, 1, 1, 1);
            } break;

            case 2:
            {
                color = v4(1, 1, 0, 1);
            } break;

            case 3:
            {
                color = v4(0.5f, 0, 0.5f, 1);
            } break;

            case 4:
            {
                color = v4(0, 0, 1, 1); 
            } break;

            case 5:
            {
                color = v4(1, 0.59f, 0.11f, 1);
            } break;

            case 6:
            {
                color = v4(0, 1, 0, 1);
            } break;

            case 7:
            {
                color = v4(1, 0, 0, 1);
            } break;

            default:
            {
                color = v4(0, 0, 0, 0);
            } break;
        }
    }

    return color;
}

INIT_APP(Init)
{
    Assert(sizeof(app_state) <= platform->storage_size);
    state = (app_state *)platform->storage;

    srand(time(0));
    InitRenderer(&state->renderer);

    state->current_piece.type = (tetromino_type)(rand() % 7);
    state->current_piece.row_offset = 0;
    state->current_piece.col_offset = WIDTH / 2;
        
    platform->initialized = 1;
}

UPDATE_APP(Update)
{
    // Update
    state->delta_t = platform->current_time - platform->last_time;
    state->time_since_last_drop += (state->delta_t);

    if (!state->game_over)
    {
        local_persist u8 count = 0;
        local_persist b32 left_valid = 1; 
        local_persist b32 right_valid = 1; 

        piece test_piece = state->current_piece;

        // Movement on test piece
        if (platform->key_down[KEY_left] && left_valid)
        {
            --test_piece.col_offset;
            left_valid = 0;
        }

        if (platform->key_down[KEY_right] && right_valid)
        {
            ++test_piece.col_offset;
            right_valid = 0;
        }

        if (platform->key_release[KEY_up])
        {
            test_piece.rotation = (test_piece.rotation + 1) % 4;
        }

        // Check piece valid
        if (CheckPieceValid(&test_piece, state->board, WIDTH, HEIGHT))
        {
            state->current_piece = test_piece;
        }

        if (platform->key_down[KEY_down])
        {
            SoftDropPiece(state);
        }

        // Hard Drop 
        if (platform->key_release[KEY_space])
        {
            while(SoftDropPiece(state));
        }

        if (state->time_since_last_drop >= 450)
        {
            SoftDropPiece(state);
            state->time_since_last_drop = 0;
        }

        // Control piece move rate left/right
        ++count;
        if (count >= 4)
        {
            count = 0;
            left_valid = 1;
            right_valid = 1;
        }
    }

    // Render
    ClearScreen(v4(0.2f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    // Render Grid
    for (int i = 0; i <= WIDTH; ++i)
    {
        RenderRect(&state->renderer, v2(i * CELL_SIZE, 0),
                   v2(1, HEIGHT * CELL_SIZE), v4(.5, .5, .5, 1));
    }

    for (int i = 0; i <= HEIGHT; ++i)
    {
        RenderRect(&state->renderer, v2(0, i * CELL_SIZE),
                   v2(WIDTH * CELL_SIZE, 1), v4(.5, .5, .5, 1));
    }

    int piece_size = tetrominos[state->current_piece.type].size;

    int pitch = state->current_piece.row_offset * WIDTH +
        state->current_piece.col_offset;

    tetromino *tetro = &tetrominos[state->current_piece.type];

    // Render Copy Piece
    piece copy_piece = state->current_piece;
    // copy_piece.row_offset = HEIGHT - tetro->size + 1; 

    while (CheckPieceValid(&copy_piece, state->board, WIDTH, HEIGHT))
    {
        ++copy_piece.row_offset;
    }

    --copy_piece.row_offset;

    for (int row = 0; row < tetro->size; ++row)
    {
        for (int col = 0; col < tetro->size; ++col)
        {
            u8 val = GetTetrominoData(tetro, row, col,
                                      copy_piece.rotation);

            if (val)
            {
                int board_row = copy_piece.row_offset + row;
                int board_col = copy_piece.col_offset + col;

                v2 pos = v2(board_col * CELL_SIZE, board_row * CELL_SIZE);
                v2 size = v2(CELL_SIZE, CELL_SIZE); 

                v4 color = v4(0.5f, 0.5f, 0.5f, 1);
                RenderRect(&state->renderer, pos, size, color);
            }
        }
    }

    // Render current piece
    for (int row = 0; row < tetro->size; ++row)
    {
        for (int col = 0; col < tetro->size; ++col)
        {
            u8 val = GetTetrominoData(tetro, row, col,
                                      state->current_piece.rotation);


            if (val)
            {
                int board_row = state->current_piece.row_offset + row;
                int board_col = state->current_piece.col_offset + col;

                v2 pos = v2(board_col * CELL_SIZE, board_row * CELL_SIZE);
                v2 size = v2(CELL_SIZE, CELL_SIZE); 

                v4 color = GetColor(val, state->game_over);
                RenderRect(&state->renderer, pos, size, color);
            }
        }
    }

    // Render board
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        if (state->board[i])
        {
            int col = i % WIDTH;
            int row = (i - col) / WIDTH;

            v2 pos = v2(col * CELL_SIZE, row * CELL_SIZE);
            v2 size = v2(CELL_SIZE, CELL_SIZE);

            v4 color = GetColor(state->board[i], state->game_over);
            RenderRect(&state->renderer, pos, size, color);
        }
    } 

    // Render Banner
    RenderRect(&state->renderer, v2(1, 0),
               v2(WIDTH * CELL_SIZE - 1, 2 * CELL_SIZE),
               v4(0.2f, 0.3f, 0.3f, 1.0f));

    // T
    RenderRect(&state->renderer, v2(10, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(27, 5), v2(5, 50), v4(1, 1, 1, 1));

    // E
    RenderRect(&state->renderer, v2(58, 5), v2(5, 50), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(58, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(58, 28), v2(32, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(58, 50), v2(38, 5), v4(1, 1, 1, 1));
                                                                            
    // T
    RenderRect(&state->renderer, v2(106, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(123, 5), v2(5, 50), v4(1, 1, 1, 1));

    // R
    RenderRect(&state->renderer, v2(154, 5), v2(5, 50), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(154, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(154, 28), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(187, 5), v2(5, 28), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(182, 28), v2(5, 12), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(187, 38), v2(5, 18), v4(1, 1, 1, 1));

    // I
    RenderRect(&state->renderer, v2(219, 5), v2(5, 50), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(202, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(202, 50), v2(38, 5), v4(1, 1, 1, 1));

    // S
    RenderRect(&state->renderer, v2(250, 5), v2(5, 25), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(250, 5), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(250, 28), v2(38, 5), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(283, 28), v2(5, 25), v4(1, 1, 1, 1));
    RenderRect(&state->renderer, v2(250, 50), v2(38, 5), v4(1, 1, 1, 1));

    SubmitRenderer(&state->renderer);
    platform->SwapBuffers();
}

DRIFT_MAIN(DriftMain)
{
    drift_application app = {0};
    {
        app.name = "Tetris";
        app.window_width = 317;
        app.window_height = 700;
        app.window_style = (DWS_overlapped | DWS_caption | DWS_sysmenu |
                            DWS_minimizebox | DWS_maximizebox);
    }

    platform = platform_;
    return app;
}

