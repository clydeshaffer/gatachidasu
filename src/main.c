#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/audio/music.h"
#include "gt/gfx/sprites.h"
#include "gt/input.h"
#include "gt/banking.h"
#include <zlib.h>

#include "grid.h"
#include "game_timer.h"
#include "mode_menu.h"
#include "tutorial.h"

#include "gen/assets/music.h"
#include "gen/assets/bg.h"
#include "gen/assets/bg/player.json.h"

#include "sine_tables.h"
#include <zlib.h>


SpriteSlot bgImg;
SpriteSlot playerImg;
SpriteSlot titleImg;
SpriteSlot finishImg;
SpriteSlot bitsImg;
SpriteSlot modeMenuImg;
SpriteSlot tutorialImg;
SpriteSlot bossTitlesImg;

#define START_LIVES_COUNT 3

#define ROTATION_ANGLE 32
char rotation_direction = 0;
char rotation_timer = 0;

#define PLAYER_NORMAL_Y 107
#define PLAYER_SUBFRAMES 6

#define GAME_STATE_TITLE 0
#define GAME_STATE_PLAYING 1
#define GAME_STATE_FINISH 2

char player_x;
signed char player_y;
char player_vy;
char target_x;
char player_frame, player_frame_start, player_frame_end;
char player_frame_end_next;
char player_subframe;

char win_state;
char prev_win_state;
char game_state;
int puzzle_offset = 0;
char boss_counter = 0;
char boss_num;

char global_tick = 0;
char lives;

static const char title_colors[8] = {118, 182, 23, 238, 118, 182, 23, 238};
char color_cycle = 0;

int main () {
    init_graphics();

    direct_prepare_array_mode();
    change_rom_bank(ASSET__bg__splash_bmp_bank);
    *bank_reg = 0;
    inflatemem(vram+0x2000, &ASSET__bg__splash_bmp_ptr);
    init_graphics();

    init_game_timer_system();
    bgImg = allocate_sprite(&ASSET__bg__scene_bmp_load_list);
    bitsImg = allocate_sprite(&ASSET__bg__bits_bmp_load_list);
    titleImg = allocate_sprite(&ASSET__bg__title_bmp_load_list);
    modeMenuImg = allocate_sprite(&ASSET__bg__modes_bmp_load_list);
    finishImg = allocate_sprite(&ASSET__bg__time_attack_finish_bmp_load_list);
    playerImg = allocate_sprite(&ASSET__bg__player_bmp_load_list);
    set_sprite_frametable(playerImg, (const Frame*)ASSET__bg__player_json);
    tutorialImg = allocate_sprite(&ASSET__bg__buttons_bmp_load_list);
    set_sprite_frametable(tutorialImg, (const Frame*)ASSET__bg__buttons_json);
    bossTitlesImg = allocate_sprite(&ASSET__bg__boss_titles_bmp_load_list);
    set_sprite_frametable(bossTitlesImg, (const Frame*)ASSET__bg__boss_titles_json);
    game_state = GAME_STATE_TITLE;


    while(1) {
        puzzle_offset = 0;
        boss_counter = 1;
        boss_num = 0;
        play_song(ASSET__music__kachispond_mid, REPEAT_LOOP);
        global_tick = 0;
        grid_init(bitsImg);
        thumbnail_enabled = 0;
        grid_x_pos = 24;
        grid_y_pos = 80;
        player_frame = PLAYER_TAG_STEP_RIGHT_START;
        while(game_state == GAME_STATE_TITLE) {
            grid_rotation += global_tick&1;
            queue_draw_box(0, 0, 127, 68, 180);
            setSineMode(0);
            player_vy = getSine(global_tick+16);
            player_vy += 8;
            player_vy >>= 2;
            queue_draw_tiled(0, 64 + player_vy, 127, 16, 112, 64, modeMenuImg);
            queue_draw_tiled(0, 80 + player_vy, 127, 16, 112, 80, modeMenuImg);
            queue_draw_tiled(0, 96 + player_vy, 127, 16, 112, 96, modeMenuImg);
            queue_draw_tiled(0, 112 + player_vy, 127, 16, 112, 112, modeMenuImg);
            queue_draw_sprite(20, 10, 88, 47, 20, 10, titleImg);
            player_vy = getSine(global_tick);
            player_vy += 8;
            player_vy >>= 2;
            queue_draw_sprite(27, 78 + player_vy, 73, 38, 3, 85, titleImg);

            queue_draw_sprite_frame(playerImg, 32, PLAYER_NORMAL_Y, player_frame, 0);
            ++player_subframe;
            if(player_subframe > 6) {
                player_subframe = 0;
                ++player_frame;
                if(player_frame >= PLAYER_TAG_STEP_RIGHT_END) {
                    player_frame = PLAYER_TAG_STEP_RIGHT_START;
                }
            }

            
            await_draw_queue();
            push_rom_bank();
            change_rom_bank(ASSET__bg__puzzles_bin_bank);
            grid_draw();
            pop_rom_bank();

            if(global_tick & 64) {
                queue_draw_sprite(41, 80, 46, 8, 0, 64, titleImg);
            }
            queue_clear_border(0);

            if(player1_new_buttons & INPUT_MASK_START) {
                if(do_mode_menu(modeMenuImg)) {
                    puzzle_offset = 0;
                    game_state = GAME_STATE_PLAYING;
                    troll_move_mode = 0;
                    troll_info_mode = 0;
                    if(game_mode == MODE_TIME_ATTACK) {
                        set_game_timer(151);
                        set_puzzle_counter(0, 0);
                    } else {
                        clear_game_timer();
                    }
                    if(game_mode == MODE_TUTORIAL) {
                        init_tutorial(tutorialImg);
                        set_puzzle_counter(0, 2);
                    }
                    if(game_mode == MODE_MARATHON) {
                        lives = START_LIVES_COUNT;
                    } else {
                        lives = 0;
                    }
                }
            }

            await_draw_queue();
            await_vsync(1);
            flip_pages();
            update_inputs();
            tick_music();
            ++global_tick;
        }
        thumbnail_enabled = 1;
        global_tick = 0;
        grid_init(bitsImg);
        push_rom_bank();
        change_rom_bank(ASSET__bg__puzzles_bin_bank);
        grid_setup_puzzle(ASSET__bg__puzzles_bin_ptr);
        pop_rom_bank();
        win_state = 0;
        prev_win_state = 0;

        player_x = GRID_CENTER_X;
        player_y = 0;
        player_vy = 0;
        target_x = GRID_CENTER_X;
        player_frame = PLAYER_TAG_IDLE_START;
        player_subframe = 0;
        player_frame_start = PLAYER_TAG_IDLE_START;
        player_frame_end = PLAYER_TAG_IDLE_END;
        player_frame_end_next = player_frame_end;

        stop_music();
        play_song(ASSET__music__cocek_slow_mid, REPEAT_LOOP);

        while (game_state == GAME_STATE_PLAYING) {                                     //  Run forever
            //queue_clear_screen(3);
            //queue_draw_box(0,3, 80, 123, 63);
            queue_draw_sprite(0, 3, 127, 127, 0, 3, bgImg);
            if(game_mode == MODE_TIME_ATTACK) {
                queue_draw_sprite(82, 107, 28, 7, 82, 121, bgImg);
            } else if(game_mode == MODE_TUTORIAL) {
                check_tutorial_conditions();
            }

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
                if((!rotation_timer) || ((grid_rotation+16) & (32|64)) == grid_rotation) {
                    if(!grid_angular_momentum) {
                        grid_rotation += 16;
                        grid_rotation &= (32|64);
                    }
                    player_frame = PLAYER_TAG_IDLE_START;
                    player_frame_start = PLAYER_TAG_IDLE_START;
                    player_frame_end = PLAYER_TAG_IDLE_END;
                    player_frame_end_next = player_frame_end;
                }
            } else {
                rotation_direction = 0;
                if(player1_new_buttons & INPUT_MASK_B) {
                    --rotation_direction;
                    grid_angular_momentum = 0;
                }
                if(player1_new_buttons & INPUT_MASK_A) {
                    ++rotation_direction;
                    grid_angular_momentum = 0;
                }
                if(rotation_direction) {
                    rotation_timer = ROTATION_ANGLE;
                    player_frame_start = PLAYER_TAG_ROTATE_START;
                    player_frame_end_next = PLAYER_TAG_ROTATE_END;
                    player_frame_end = PLAYER_TAG_ROTATE_END;
                    player_frame = PLAYER_TAG_ROTATE_START;
                    player_subframe = 0;
                }
            }

            if(player1_new_buttons & INPUT_MASK_LEFT) {
                target_x -= 8;
                player_frame_start = PLAYER_TAG_IDLE_START;
                player_frame_end_next = PLAYER_TAG_IDLE_END;
                player_frame_end = PLAYER_TAG_STEP_RIGHT_END;
                player_frame = PLAYER_TAG_STEP_RIGHT_START+2;
                player_subframe = 0;
            }
            if(player1_new_buttons & INPUT_MASK_RIGHT) {
                target_x += 8;
                player_frame_start = PLAYER_TAG_IDLE_START;
                player_frame_end_next = PLAYER_TAG_IDLE_END;
                player_frame_end = PLAYER_TAG_STEP_RIGHT_END;
                player_frame = PLAYER_TAG_STEP_RIGHT_START+2;
                player_subframe = 0;
            }

            if(target_x < (GRID_CENTER_X - 24)) {
                target_x = GRID_CENTER_X - 24;
            } else if(target_x > (GRID_CENTER_X + 24)) {
                target_x = GRID_CENTER_X + 24;
            }

            if(target_x == player_x) {
                if(player1_new_buttons & INPUT_MASK_C) {
                    if(~win_state & GRID_DRAW_RESULT_PRE_WIN) {
                        if(grid_send_bullet(player_x)) {
                            play_sound_effect(ASSET__music__shoot_sfx_ID, 2);
                            player_frame_start = PLAYER_TAG_IDLE_START;
                            player_frame_end_next = PLAYER_TAG_IDLE_END;
                            player_frame_end = PLAYER_TAG_BULLET_END;
                            player_frame = PLAYER_TAG_BULLET_START+2;
                            player_subframe = 0;
                        }
                    }
                }
            }
            if(target_x < player_x) player_x -= 2;
            if(target_x > player_x) player_x += 2;
            push_rom_bank();
            change_rom_bank(ASSET__bg__puzzles_bin_bank);
            win_state |= grid_draw();
            pop_rom_bank();

            queue_draw_sprite_frame(playerImg, player_x, PLAYER_NORMAL_Y + (player_y >> 2), player_frame, 0);
            if((++player_subframe) == PLAYER_SUBFRAMES) {
                player_subframe = 0;
                if((++player_frame) == player_frame_end) {
                    player_frame_end = player_frame_end_next;
                    player_frame = player_frame_start;
                }
            }
            

            if(win_state & GRID_DRAW_RESULT_WIN) {
                win_state = 0;
                puzzle_offset += GRID_FULL_COUNT;
                ++boss_counter;
                if(boss_counter == 5) {
                    boss_counter = 0;
                    ++boss_num;
                    setup_troll_modes(boss_num);
                    play_song(ASSET__music__kachi_boss_mid, REPEAT_LOOP);
                } else {
                    if(troll_title_frame != 0xFF) {
                        lives = START_LIVES_COUNT;
                        play_song(ASSET__music__cocek_slow_mid, REPEAT_LOOP);
                    }
                    setup_troll_modes(0xFF);
                }
                if(game_mode == MODE_MARATHON) {
                    if(decrement_puzzle_counter()) {
                        game_state = GAME_STATE_FINISH;
                    } else {
                        grid_init(bitsImg);
                        push_rom_bank();
                        change_rom_bank(ASSET__bg__puzzles_bin_bank);
                        grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr[puzzle_offset]);
                        pop_rom_bank();
                    }
                } else if(game_mode == MODE_TIME_ATTACK) {
                    if(puzzle_offset >= ASSET__bg__puzzles_bin_size) {
                        puzzle_offset = 0;
                    }
                    increment_puzzle_counter();
                    grid_init(bitsImg);
                    push_rom_bank();
                    change_rom_bank(ASSET__bg__puzzles_bin_bank);
                    grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr[puzzle_offset]);
                    pop_rom_bank();
                } else if(game_mode == MODE_TUTORIAL) {
                    if(decrement_puzzle_counter()) {
                        game_state = GAME_STATE_FINISH;
                        troll_move_mode = 0;
                    }
                    if(boss_counter == 2) {
                        setup_troll_modes(0);
                    }
                    if(boss_counter < 3) {
                        grid_init(bitsImg);
                        push_rom_bank();
                        change_rom_bank(ASSET__bg__puzzles_bin_bank);
                        grid_setup_puzzle(&ASSET__bg__puzzles_bin_ptr[puzzle_offset]);
                        pop_rom_bank();
                    }
                }
               
            } else if(win_state & GRID_DRAW_RESULT_LOSE) {
                //queue_draw_box(65,33, 16, 16, 90);
                win_state = 0;
                if(game_mode == MODE_MARATHON) {
                    --lives;
                    if(!lives) {
                        game_state = GAME_STATE_FINISH;
                    }
                }
            }
            
            if(win_state & GRID_DRAW_RESULT_PRE_WIN) {
                player_frame = PLAYER_TAG_IDLE_START;
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

            render_game_timer();

            if(game_mode == MODE_TUTORIAL) {
                draw_tutorial();
            }

            if(game_mode == MODE_MARATHON) {
                if(lives) {
                    queue_draw_tiled(84, 11, (lives << 3)+1, 8, 96+7, 0, bitsImg);
                }
            }

            if(troll_title_frame != 0xFF) {
                queue_draw_sprite_frame(bossTitlesImg, GRID_CENTER_X, 16, troll_title_frame, 0);
            }

            queue_clear_border(0);

            await_draw_queue();
            await_vsync(1);
            flip_pages();
            ++global_tick;
            update_inputs();
            tick_music();
            if(~win_state & GRID_DRAW_RESULT_PRE_WIN) {
                if(game_mode == MODE_TIME_ATTACK) {
                    if(downtick_game_timer()) {
                        game_state = GAME_STATE_FINISH;
                    }
                } else if (game_mode == MODE_MARATHON) {
                    tick_game_timer();
                }
            }
            prev_win_state = win_state;
        }

        stop_music();
        play_song(ASSET__music__Kachi_Wins_mid, REPEAT_LOOP);

        if(game_mode == MODE_MARATHON) {
            if(lives) {
                game_timer_pos_x = 23;
                game_timer_pos_y = 32;
            } else {
                stop_music();
                play_song(ASSET__music__kachilost_mid, REPEAT_LOOP);
            }
        } else if(game_mode == MODE_TIME_ATTACK) {
            puzzle_counter_pos_x = 17;
            puzzle_counter_pos_y = 35;
        }

        while(game_state == GAME_STATE_FINISH) {
         
            ++global_tick;
            //--target_x;
            queue_draw_sprite(0, 3, 127, 127, 0, 3, bgImg);
            if(game_mode == MODE_TIME_ATTACK) {
                queue_draw_sprite(82, 107, 28, 7, 82, 121, bgImg);
            }
            push_rom_bank();
            change_rom_bank(ASSET__bg__puzzles_bin_bank);
            win_state != grid_draw();
            pop_rom_bank();
            
            rect.x = 5;
            rect.y = 21;
            rect.w = 66;
            rect.h = 64;
            rect.gx = 0;
            rect.gy = 0;
            rect.b = finishImg;
            if(game_mode == MODE_MARATHON) {
                if(lives) {
                    queue_draw_sprite_rect();
                } else {
                    rect.gx = 64;
                    rect.h = 32;
                    rect.gy = 96;
                    rect.w = 64;
                    queue_draw_sprite_rect();
                    rect.gx = 0;
                    rect.y += 32;
                    rect.gy = 32;
                    rect.b = finishImg;
                    rect.w = 66;
                    queue_draw_sprite_rect();
                }
            } else if(game_mode == MODE_TIME_ATTACK) {
                rect.h = 32;
                rect.gy = 64;
                queue_draw_sprite_rect();
                rect.y += 32;
                rect.gy = 32;
                rect.b = finishImg;
                queue_draw_sprite_rect();
            } else if(game_mode == MODE_TUTORIAL) {
                rect.h = 32;
                rect.gy = 96;
                queue_draw_sprite_rect();
                rect.y += 32;
                rect.gy = 32;
                rect.b = finishImg;
                queue_draw_sprite_rect();
            }
            render_game_timer();            
            queue_draw_sprite_frame(playerImg, player_x, PLAYER_NORMAL_Y + (player_y >> 2), player_frame, 0);
            ++player_subframe;
            if(player_subframe > 6) {
                player_subframe = 0;
                ++player_frame;
                if((game_mode == MODE_MARATHON) && !lives) {
                    if(player_frame >= PLAYER_TAG_BULLET_END) {
                        player_frame = PLAYER_TAG_BULLET_START;
                    }
                }else if(player_frame >= PLAYER_TAG_STEP_RIGHT_END) {
                    player_frame = PLAYER_TAG_STEP_RIGHT_START;
                }
            }
            queue_clear_border(0);

            if(player1_new_buttons & INPUT_MASK_START) {
                game_state = GAME_STATE_TITLE;
                clear_game_timer();
            }

            await_draw_queue();
            await_vsync(1);
            flip_pages();
            update_inputs();
            tick_music();
        }
        setup_troll_modes(0xFF);
        grid_init(bitsImg);
    }
}
