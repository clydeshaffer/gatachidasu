#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/gfx/sprites.h"

#include "gt/audio/music.h"
#include "gen/assets/music.h"

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

#define GRID_MODE_GAME 0
#define GRID_MODE_EXPLODE 1
#define GRID_MODE_DISPLAY 2
#define DISPLAY_MODE_TIME 90

char grid_render_mode;
char grid_status[GRID_FULL_COUNT];
char grid_target[GRID_FULL_COUNT];
char solution_rotations_mask;
char blocks_remaining;
char target_block_count;
char display_mode_timeout;

#define BULLET_MAX 5
char next_bullet;
char active_bullets;
char bullets_x[BULLET_MAX];
char bullets_y[BULLET_MAX];

extern char global_tick;

//These coordinates are only used in explode mode
//Otherwise blocks are drawn with polar coords
char grid_explode_x[GRID_FULL_COUNT];
char grid_explode_y[GRID_FULL_COUNT];
signed char grid_explode_vx[GRID_FULL_COUNT];
signed char grid_explode_vy[GRID_FULL_COUNT];

char grid_get_display_rotation() {
    if((1 << (grid_rotation >> 5)) & solution_rotations_mask) return (grid_rotation & 96);

    if(solution_rotations_mask & 1) {
        return 0;
    }
    if(solution_rotations_mask & 2) {
        return 32;
    }
    if(solution_rotations_mask & 4) {
        return 64;
    }
    return 96;
}

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
    grid_render_mode = GRID_MODE_GAME;
    solution_rotations_mask = 0b1111;
}

void grid_setup_explode() {
    static char i, grid_rotation_cosine;
    grid_rotation_cosine = grid_rotation+32;
    for(i = 0; i < GRID_FULL_COUNT; ++i) {
        if(grid_status[i]) {
            setSineMode(grid_radius[i]);
            grid_explode_y[i] = getSine(grid_angles[i] + grid_rotation);
            grid_explode_x[i] = getSine(grid_angles[i] + grid_rotation_cosine);
            grid_explode_vy[i] = (((signed char) grid_explode_y[i]) >> 3) - 2;
            grid_explode_vx[i] = (((signed char) grid_explode_x[i]) >> 3) + (i & 7) - 4;
            grid_explode_y[i] += GRID_SQUARE_OFFSET + grid_y_pos;
            grid_explode_x[i] += GRID_SQUARE_OFFSET + GRID_CENTER_X;
            grid_explode_y[i] <<= 1;
            grid_explode_x[i] <<= 1;
        }
    }
    grid_render_mode = GRID_MODE_EXPLODE;
}

void grid_setup_puzzle(const unsigned char *shape) {
    static char i, r, c;
    

    for(i = 0; i < GRID_FULL_COUNT; ++i) {
        grid_status[i] = 0b10000;
    }

    if(shape != (void *)0xFFFF) {
        target_block_count = 0;
        for(i = 0; i < GRID_FULL_COUNT; ++i) {
            grid_target[i] = shape[i] ? 0xFF : 0;
            if(grid_target[i]) ++target_block_count;
        }
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

char grid_send_bullet(char x) {
    if(active_bullets == BULLET_MAX) return 0;
    ++active_bullets;
    bullets_x[next_bullet] = x;
    bullets_y[next_bullet] = 104;
    if(++next_bullet == BULLET_MAX) next_bullet = 0;
    return 1;
}

char grid_draw() {
    static char r, c, i, grid_rotation_cosine, grid_ind;
    static char result;
    static signed char x, y;

    result = GRID_DRAW_RESULT_NONE;

    grid_rotation_cosine = grid_rotation + 32;

    grid_ind = 0;

    for(i = 0; i < BULLET_MAX; ++i) {
        if(bullets_x[i]) {
            bullets_y[i] -= 2;
            if(bullets_y[i] == 0) {
                bullets_x[i] = 0;
                --active_bullets;
            }
        }
    }

    if(grid_render_mode != GRID_MODE_EXPLODE) {
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
                    
                    if(grid_render_mode == GRID_MODE_GAME) {
                        for(i = 0; i < BULLET_MAX; ++i) {
                            if(bullets_x[i]) {
                                if(((char)(bullets_x[i] - x - GRID_SQUARE_OFFSET)) <= GRID_SQUARE_SIZE) {
                                    if(((char)(bullets_y[i] - y - GRID_SQUARE_OFFSET)) <= GRID_SQUARE_SIZE) {
                                        bullets_x[i] = 0;
                                        bullets_y[i] = 0;
                                        --active_bullets;
                                        solution_rotations_mask &= ~grid_status[grid_ind];
                                        grid_status[grid_ind] = 0;
                                        play_sound_effect(ASSET__music__hit_sfx_ID, 3);
                                        --blocks_remaining;
                                        if(solution_rotations_mask == 0) {
                                            grid_setup_explode();
                                            play_sound_effect(ASSET__music__break_sfx_ID, 3);
                                            result = GRID_DRAW_RESULT_LOSE;
                                        } else if(blocks_remaining == target_block_count) {
                                            grid_render_mode = GRID_MODE_DISPLAY;
                                            result = GRID_DRAW_RESULT_PRE_WIN;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                ++grid_ind;
            }
        }
    }

    direct_prepare_sprite_mode(grid_sprite);
    
    grid_rotation_cosine = grid_rotation + 32;

    grid_ind = 0;

    if(grid_render_mode == GRID_MODE_EXPLODE) {
        for(r = 0; r < GRID_SIZE; ++r) {
            for(c = 0; c < GRID_SIZE; ++c) {
                if(grid_status[grid_ind]) {
                    DIRECT_DRAW_SPRITE(grid_explode_x[grid_ind] >> 1, grid_explode_y[grid_ind] >> 1, GRID_SQUARE_SIZE, GRID_SQUARE_SIZE, GRID_SPRITE_X, GRID_SPRITE_Y);
                    
                    grid_explode_y[grid_ind] += grid_explode_vy[grid_ind];
                    if((global_tick & 3) == 0) {
                        ++grid_explode_vy[grid_ind];
                        grid_explode_x[grid_ind] += grid_explode_vx[grid_ind];
                    }
                    if((grid_explode_y[grid_ind] > 200) ||
                       (grid_explode_x[grid_ind] > 152) ||
                       (grid_explode_x[grid_ind] < 8)
                    ) {
                        grid_status[grid_ind] = 0;
                        --blocks_remaining;
                    }
                    
                    
                }
                ++grid_ind;
            }
        }
        if(blocks_remaining == 0) {
            grid_init(grid_sprite);
            grid_setup_puzzle(GRID_RESET_PUZZLE);
        }
    } else {
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
                    
                    if(grid_status[grid_ind]) {
                        x += GRID_SQUARE_OFFSET;
                        y += GRID_SQUARE_OFFSET;
                        DIRECT_DRAW_SPRITE(x, y, GRID_SQUARE_SIZE, GRID_SQUARE_SIZE, ((grid_ind == 12) ? GRID_SPRITE_CENTER_X : GRID_SPRITE_X), GRID_SPRITE_Y);
                    }
                }
                ++grid_ind;
            }
        }
    }
  
    if(grid_render_mode == GRID_MODE_GAME) {
        for(i = 0; i < BULLET_MAX; ++i) {
            if(bullets_x[i]) {
                DIRECT_DRAW_SPRITE(bullets_x[i] - 2, bullets_y[i] - 2, 4, 4, 39, 95);
            }
        }
    } else if(grid_render_mode == GRID_MODE_DISPLAY) {
        if(display_mode_timeout) {
            grid_rotation = grid_get_display_rotation() + (sineRadius8[(((DISPLAY_MODE_TIME-1) - display_mode_timeout) << 2) & 0x7F] >> 1);
            --display_mode_timeout;
            if(display_mode_timeout == 0) {
                result = GRID_DRAW_RESULT_WIN;
            }
        } else {
            if(((grid_rotation & 0x7E) == grid_get_display_rotation())) {
                display_mode_timeout = DISPLAY_MODE_TIME;
            } else {
                grid_rotation += 2;
            }
        }
    }

    grid_ind = 0;
    y = 38;
    for(r = 0; r < GRID_SIZE; ++r) {
        x = 87;
        for(c = 0; c < GRID_SIZE; ++c) {
            if(grid_target[grid_ind] & 1) {
                if(grid_ind == 12) {
                    DIRECT_DRAW_SPRITE(x, y, 7, 7, 101, 52);
                } else {
                    DIRECT_DRAW_SPRITE(x, y, 7, 7, 87, 38);
                }
                
            } else {
                DIRECT_DRAW_SPRITE(x, y, 7, 7, 101, 38);
            }
            x += 7;
            ++grid_ind;
        }
        y += 7;
    }
    //await_drawing();
    return result;
}