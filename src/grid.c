#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/gfx/sprites.h"

#include "grid.h"
#include "sine_tables.h"

char grid_rotation;
char grid_y_pos;
SpriteSlot grid_sprite;

const char grid_angles[25] = {
    -48, -41, -32, -23, -16,
    -55, -48, -32, -16, -9,
    64, 64, 0, 0, 0,
    55, 48, 32, 16, 9,
    48, 41, 32, 23, 16
};

const char grid_radius[25] = {
    4, 3, 2, 3, 4,
    3, 1, 0, 1, 3,
    2, 0,-1, 0, 2,
    3, 1, 0, 1, 3,
    4, 3, 2, 3, 4
};

char grid_status[25];

#define BULLET_MAX 3
char next_bullet = 0;
char bullets_x[BULLET_MAX];
char bullets_y[BULLET_MAX];

void grid_init(SpriteSlot s) {
    static char i;
    grid_rotation = 0;
    grid_y_pos = GRID_CENTER_Y_START;
    grid_sprite = s;
    for(i = 0; i < BULLET_MAX; ++i) {
        bullets_x[i] = 0;
        bullets_y[i] = 0;
    }
    for(i = 0; i < 25; ++i) {
        grid_status[i] = 0b10000;
    }
}

void grid_send_bullet(char x) {
    bullets_x[next_bullet] = x;
    bullets_y[next_bullet] = 104;
    if(++next_bullet == BULLET_MAX) next_bullet = 0;
}

void grid_draw() {
    static char r, c, i, grid_rotation_cosine, grid_ind;
    signed char x, y;

    direct_prepare_sprite_mode(grid_sprite);
    
    grid_rotation_cosine = grid_rotation + 32;

    for(i = 0; i < BULLET_MAX; ++i) {
        if(bullets_x[i]) {
            bullets_y[i] -= 2;
            if(bullets_y[i] == 0) {
                bullets_x[i] = 0;
            }
        }
    }

    grid_ind = 0;

    for(r = 0; r < GRID_SIZE; ++r) {
        for(c = 0; c < GRID_SIZE; ++c) {
            if(grid_status[grid_ind]) {
                if(grid_radius[grid_ind] & 0x80) {
                    x = 0;
                    y = 0;
                } else {
                    setSineMode(grid_radius[grid_ind]);
                    y = getSine(grid_angles[grid_ind] + grid_rotation);
                    x = getSine(grid_angles[grid_ind] + grid_rotation_cosine);
                }

                x += GRID_CENTER_X;
                y += grid_y_pos;

                for(i = 0; i < BULLET_MAX; ++i) {
                    if(bullets_x[i]) {
                        if(((char)(bullets_x[i] - x - GRID_SQUARE_OFFSET)) <= GRID_SQUARE_SIZE) {
                            if(((char)(bullets_y[i] - y - GRID_SQUARE_OFFSET)) <= GRID_SQUARE_SIZE) {
                                bullets_x[i] = 0;
                                bullets_y[i] = 0;
                                grid_status[grid_ind] = 0;
                            }
                        }
                    }
                }

                if(grid_status[grid_ind]) {
                    x += GRID_SQUARE_OFFSET;
                    y += GRID_SQUARE_OFFSET;
                    DIRECT_DRAW_SPRITE(x, y, GRID_SQUARE_SIZE, GRID_SQUARE_SIZE, GRID_SPRITE_X, GRID_SPRITE_Y);
                }
            }
            ++grid_ind;
        }
    }

    for(i = 0; i < BULLET_MAX; ++i) {
        if(bullets_x[i]) {
            DIRECT_DRAW_SPRITE(bullets_x[i] - 2, bullets_y[i] - 2, 4, 4, 39, 95);
        }
    }

    //await_drawing();
}