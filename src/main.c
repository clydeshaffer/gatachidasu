#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/audio/music.h"
#include "gt/gfx/sprites.h"
#include "gt/input.h"
#include "gt/banking.h"
#include <zlib.h>

#include "grid.h"

#include "gen/assets/music.h"
#include "gen/assets/bg.h"

SpriteSlot bgImg;

#define ROTATION_ANGLE 32
char rotation_direction = 0;
char rotation_timer = 0;

#define PLAYER_NORMAL_Y 103
#define PLAYER_SPRITE_X 33
#define PLAYER_SPRITE_Y 103

char player_x;
signed char player_y;
char player_vy;
char target_x;

char win_state;
char prev_win_state;

int puzzle_offset = 0;

char global_tick = 0;

int main () {
    init_graphics();
    bgImg = allocate_sprite(&ASSET__bg__scene_bmp_load_list);

    grid_init(bgImg);
    push_rom_bank();
    change_rom_bank(ASSET__bg__puzzles_bin_bank);
    grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr);
    pop_rom_bank();
    win_state = 0;
    prev_win_state = 0;

    player_x = GRID_CENTER_X;
    player_y = 0;
    player_vy = 0;
    target_x = GRID_CENTER_X;

    play_song(ASSET__music__katachidasu_mid, REPEAT_LOOP);

    while (1) {                                     //  Run forever
        //queue_clear_screen(3);
        queue_draw_box(0,3, 80, 123, 63);
        queue_draw_sprite(80, 3, 48, 123, 80, 3, bgImg);

        if(rotation_timer) {

            if(player1_new_buttons & INPUT_MASK_B) {
                if(rotation_direction == 1) {
                    rotation_direction = -1;
                    rotation_timer = ROTATION_ANGLE - rotation_timer;
                }
            }
            if(player1_new_buttons & INPUT_MASK_A) {
                if(rotation_direction == 255) {
                    rotation_direction = 1;
                    rotation_timer = ROTATION_ANGLE - rotation_timer;
                }
            }

            grid_rotation += rotation_direction;
            --rotation_timer;
            if(!rotation_timer) {
                grid_rotation &= (32|64);
            }
        } else {
            rotation_direction = 0;
            if(player1_new_buttons & INPUT_MASK_B) {
                --rotation_direction;
            }
            if(player1_new_buttons & INPUT_MASK_A) {
                ++rotation_direction;
            }
            if(rotation_direction) {
                rotation_timer = ROTATION_ANGLE;
            }
        }

        if(player1_new_buttons & INPUT_MASK_LEFT) {
            target_x -= 8;
        }
        if(player1_new_buttons & INPUT_MASK_RIGHT) {
            target_x += 8;
        }

        if(target_x < (GRID_CENTER_X - 16)) {
            target_x = GRID_CENTER_X - 16;
        } else if(target_x > (GRID_CENTER_X + 16)) {
            target_x = GRID_CENTER_X + 16;
        }

        if(target_x == player_x) {
            if(player1_new_buttons & INPUT_MASK_C) {
                grid_send_bullet(player_x);
            }
        }
        if(target_x < player_x) player_x -= 2;
        if(target_x > player_x) player_x += 2;

        win_state |= grid_draw();

        if(player1_new_buttons & INPUT_MASK_START) {
            grid_init(bgImg);
            push_rom_bank();
            change_rom_bank(ASSET__bg__puzzles_bin_bank);
            grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr[puzzle_offset]);
            win_state = 0;
        }

        queue_draw_sprite(player_x - 8, PLAYER_NORMAL_Y + (player_y >> 2), 16, 16, PLAYER_SPRITE_X, PLAYER_SPRITE_Y, bgImg);

        if(win_state & GRID_DRAW_RESULT_WIN) {
            //queue_draw_box(65,33, 16, 16, 251);
            win_state = 0;
            puzzle_offset += GRID_FULL_COUNT;
            if(puzzle_offset >= ASSET__bg__puzzles_bin_size) {
                puzzle_offset = 0;
            }
            grid_init(bgImg);
            push_rom_bank();
            change_rom_bank(ASSET__bg__puzzles_bin_bank);
            grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr[puzzle_offset]);
            pop_rom_bank();
        } else if(win_state & GRID_DRAW_RESULT_LOSE) {
            //queue_draw_box(65,33, 16, 16, 90);
            win_state = 0;
        }
        
        if(win_state & GRID_DRAW_RESULT_PRE_WIN) {
            if((~prev_win_state) & GRID_DRAW_RESULT_PRE_WIN) {
                play_sound_effect(ASSET__music__correct_sfx_ID, 2);
            }
            if(player_y >= 0) {
                player_vy = -10;
                player_y = 0;
            }
            player_y += player_vy;
            player_vy += 1;
        } else {
            player_y = 0;
            player_vy = 0;
        }

        queue_clear_border(0);

        await_draw_queue();
        await_vsync(1);
        flip_pages();
        ++global_tick;
        update_inputs();
        tick_music();
        prev_win_state = win_state;
    }
 
  return (0);                                     //  We should never get here!
}