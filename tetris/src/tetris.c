#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "drift_renderer.c"
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
            }
        }
    }
}

internal void SpawnNewPiece(app_state *game_state)
{
    game_state->current_piece.type = TETROMINO_t;
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
        SpawnNewPiece(game_state);
        return 0;
    }

    return 1;
}

internal void InitApp()
{
    Assert(sizeof(app_state) <= platform->storage_size);
    state = (app_state *)platform->storage;

    InitRenderer(&state->renderer);

    state->current_piece.type = TETROMINO_t;
    state->current_piece.row_offset = 0;
    state->current_piece.col_offset = WIDTH / 2;
        
    platform->initialized = 1;
}

internal void UpdateApp()
{
    // Update
    state->delta_t = platform->current_time - platform->last_time;

    piece test_piece = state->current_piece;
    
    if (platform->key_down[KEY_left])
    {
        --test_piece.col_offset;
    }

    if (platform->key_down[KEY_right])
    {
        ++test_piece.col_offset;
    }

    if (platform->key_down[KEY_up])
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
    if (platform->key_down[KEY_space])
    {
        while(SoftDropPiece(state));
    }
    
    if (state->delta_t >= 1000)
    {
        SoftDropPiece(state);
    }

    // Render
    ClearScreen(v4(0.2f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    int piece_size = tetrominos[state->current_piece.type].size;

    int pitch = state->current_piece.row_offset * WIDTH +
        state->current_piece.col_offset;

    // Render current piece
    tetromino *tetro = &tetrominos[state->current_piece.type];

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

                RenderRect(&state->renderer, pos, size, v4(1, 1, 1, 1));
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

            RenderRect(&state->renderer, pos, size, v4(1, 1, 1, 1));
        }
    } 

    SubmitRenderer(&state->renderer);
    platform->SwapBuffers();
}

internal drift_application DriftMain()
{
    drift_application app = {0};
    {
        app.name = "Tetris";
        app.window_width = 316;
        app.window_height = 700;

        app.Init = InitApp;
        app.Update = UpdateApp;
    }

    return app;
}

