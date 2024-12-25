#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/audio/music.h"
#include "gen/assets/music.h"
#include "gen/assets/bg.h"
#include "gt/gfx/sprites.h"
#include "gt/input.h"

#include "grid.h"

SpriteSlot bgImg;

#define ROTATION_ANGLE 32
char rotation_direction = 0;
char rotation_timer = 0;

char player_x;

char target_x;

int main () {
 
    init_graphics();
    bgImg = allocate_sprite(&ASSET__bg__scene_bmp_load_list);

    grid_init(bgImg);
    player_x = GRID_CENTER_X;
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

        grid_draw();

        if(player1_new_buttons & INPUT_MASK_START) {
            grid_init(bgImg);
        }

        queue_draw_sprite(player_x - 8, 103, 16, 16, 33, 103, bgImg);

        queue_clear_border(0);

        await_draw_queue();
        await_vsync(1);
        flip_pages();
        update_inputs();
        tick_music();
    }
 
  return (0);                                     //  We should never get here!
}