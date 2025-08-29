#include "gt/gfx/sprites.h"
#include "gt/gfx/draw_queue.h"
#include "game_timer.h"
#include "gen/assets/bg.h"

game_time current_game_time;
char game_time_hit_cap;
SpriteSlot timer_font;
char game_timer_pos_x;
char game_timer_pos_y;

char puzzle_counter_ones;
char puzzle_counter_tens;

char puzzle_counter_pos_x;
char puzzle_counter_pos_y;

#define DIGIT_WIDTH 6
#define DIGIT_HEIGHT 9
#define MS_SPACING 5
static const char number_offsets[10] = {
    DIGIT_WIDTH * 0,
    DIGIT_WIDTH * 1,
    DIGIT_WIDTH * 2,
    DIGIT_WIDTH * 3,
    DIGIT_WIDTH * 4,
    DIGIT_WIDTH * 5,
    DIGIT_WIDTH * 6,
    DIGIT_WIDTH * 7,
    DIGIT_WIDTH * 8,
    DIGIT_WIDTH * 9
};

void init_game_timer_system() {
    clear_game_timer();
    timer_font = allocate_sprite(&ASSET__bg__numbers_bmp_load_list);
}

void clear_game_timer() {
    current_game_time.frames = 0;
    current_game_time.seconds_ones = 0;
    current_game_time.seconds_tens = 0;
    current_game_time.minutes_ones = 0;
    current_game_time.minutes_tens = 0;
    puzzle_counter_ones = 4;
    puzzle_counter_tens = 3;

    game_timer_pos_x = 90;
    game_timer_pos_y = 94;
    
    puzzle_counter_pos_x = 113;
    puzzle_counter_pos_y = 109;
}

void tick_game_timer() {
    if(current_game_time.frames & 128) return;
    ++current_game_time.frames;
    if(current_game_time.frames == 60) {
        current_game_time.frames = 0;
        ++current_game_time.seconds_ones;
        if(current_game_time.seconds_ones == 10) {
            current_game_time.seconds_ones = 0;
            ++current_game_time.seconds_tens;
            if(current_game_time.seconds_tens == 6) {
                current_game_time.seconds_tens = 0;
                ++current_game_time.minutes_ones;
                if(current_game_time.minutes_ones == 10) {
                    current_game_time.minutes_ones = 0;
                    ++current_game_time.minutes_tens;
                    if(current_game_time.minutes_tens == 10) {
                        current_game_time.seconds_ones = 9;
                        current_game_time.seconds_tens = 9;
                        current_game_time.minutes_ones = 9;
                        current_game_time.minutes_tens = 9;
                        current_game_time.frames = 128;
                    }
                }
            }
        }
    }
}

void render_game_timer() {
    queue_draw_sprite(game_timer_pos_x, game_timer_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[current_game_time.minutes_tens], 0, timer_font);
    queue_draw_sprite(game_timer_pos_x+number_offsets[1], game_timer_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[current_game_time.minutes_ones], 0, timer_font);
    queue_draw_sprite(game_timer_pos_x+number_offsets[2]+MS_SPACING, game_timer_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[current_game_time.seconds_tens], 0, timer_font);
    queue_draw_sprite(game_timer_pos_x+number_offsets[3]+MS_SPACING, game_timer_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[current_game_time.seconds_ones], 0, timer_font);

    queue_draw_sprite(puzzle_counter_pos_x, puzzle_counter_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[puzzle_counter_tens], 0, timer_font);
    queue_draw_sprite(puzzle_counter_pos_x+DIGIT_WIDTH, puzzle_counter_pos_y, DIGIT_WIDTH, DIGIT_HEIGHT, number_offsets[puzzle_counter_ones], 0, timer_font);
}

char tick_puzzle_counter() {
    --puzzle_counter_ones;
    if(puzzle_counter_ones == 255) {
        puzzle_counter_ones = 9;
        --puzzle_counter_tens;
        if(puzzle_counter_tens == 255) {
            puzzle_counter_tens = 0;
        }
    }
    return !(puzzle_counter_ones | puzzle_counter_tens);
}