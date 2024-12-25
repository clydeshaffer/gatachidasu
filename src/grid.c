#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/gfx/sprites.h"

#include "grid.h"
#include "sine_tables.h"

char grid_rotation;
char grid_y_pos;
SpriteSlot grid_sprite;

const char grid_angles[GRID_FULL_COUNT] = {
    -48, -41, -32, -23, -16,
    -55, -48, -32, -16, -9,
    64, 64, 0, 0, 0,
    55, 48, 32, 16, 9,
    48, 41, 32, 23, 16
};

const char grid_radius[GRID_FULL_COUNT] = {
    4, 3, 2, 3, 4,
    3, 1, 0, 1, 3,
    2, 0,-1, 0, 2,
    3, 1, 0, 1, 3,
    4, 3, 2, 3, 4
};

char grid_status[GRID_FULL_COUNT];

char grid_target[GRID_FULL_COUNT];
char solution_rotations_mask;
char blocks_remaining;
char target_block_count;

#define BULLET_MAX 5
char next_bullet;
char active_bullets;
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
    for(i = 0; i < GRID_FULL_COUNT; ++i) {
        grid_status[i] = 0b10000;
    }
    active_bullets = 0;
    next_bullet = 0;
    blocks_remaining = GRID_FULL_COUNT;
}

void grid_setup_puzzle(char* shape) {
    static char i, r, c;
    solution_rotations_mask = 0b1111;
    target_block_count = 0;

    for(i = 0; i < GRID_FULL_COUNT; ++i) {
        grid_status[i] = 0b10000;
    }

    for(i = 0; i < GRID_FULL_COUNT; ++i) {
        grid_target[i] = shape[i] ? 0xFF : 0;
        if(grid_target[i]) ++target_block_count;
    }

    //March the grid in this order for each rotation
    //R+, C+
    //C-, R+
    //R-, C-
    //C+, R-

    i = 0; //index into grid_status
    for(r = 0; r < GRID_SIZE; ++r) {
        for(c = 0; c < GRID_SIZE; ++c) {
            grid_status[i] |= grid_target[i] & 0b0001;
            ++i;
        }
    }
    i = 0; //index into grid_status
    for(c = GRID_SIZE-1; c < GRID_SIZE; --c) {
        for(r = 0; r < GRID_FULL_COUNT; r += GRID_SIZE) {
            grid_status[i] |= grid_target[r+c] & 0b0010;
            ++i;
        }
    }
    i = 0; //index into grid_status
    for(r = GRID_FULL_COUNT-GRID_SIZE; r < GRID_FULL_COUNT; r -= GRID_SIZE) {
        for(c = GRID_SIZE-1; c < GRID_SIZE; --c)  {
            grid_status[i] |= grid_target[r+c] & 0b0100;
            ++i;
        }
    }
    i = 0; //index into grid_status
    for(c = 0; c < GRID_SIZE; ++c) {
        for(r = GRID_FULL_COUNT-GRID_SIZE; r < GRID_FULL_COUNT; r -= GRID_SIZE)  {
            grid_status[i] |= grid_target[r+c] & 0b1000;
            ++i;
        }
    }
}

void grid_send_bullet(char x) {
    if(active_bullets == BULLET_MAX) return;
    ++active_bullets;
    bullets_x[next_bullet] = x;
    bullets_y[next_bullet] = 104;
    if(++next_bullet == BULLET_MAX) next_bullet = 0;
}

char grid_draw() {
    static char r, c, i, grid_rotation_cosine, grid_ind;
    static char result;
    static signed char x, y;

    result = GRID_DRAW_RESULT_NONE;

    direct_prepare_sprite_mode(grid_sprite);
    
    grid_rotation_cosine = grid_rotation + 32;

    for(i = 0; i < BULLET_MAX; ++i) {
        if(bullets_x[i]) {
            bullets_y[i] -= 2;
            if(bullets_y[i] == 0) {
                bullets_x[i] = 0;
                --active_bullets;
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
                                --active_bullets;
                                solution_rotations_mask &= ~grid_status[grid_ind];
                                grid_status[grid_ind] = 0;
                                --blocks_remaining;
                                if(solution_rotations_mask == 0) {
                                    result = GRID_DRAW_RESULT_LOSE;
                                } else if(blocks_remaining == target_block_count) {
                                    result = GRID_DRAW_RESULT_WIN;
                                }
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

    grid_ind = 0;
    y = 38;
    for(r = 0; r < GRID_SIZE; ++r) {
        x = 87;
        for(c = 0; c < GRID_SIZE; ++c) {
            if(grid_target[grid_ind] & 1) {
                DIRECT_DRAW_SPRITE(x, y, 7, 7, 87, 38);
            }
            x += 7;
            ++grid_ind;
        }
        y += 7;
    }
    //await_drawing();
    return result;
}