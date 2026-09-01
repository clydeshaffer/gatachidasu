#include "tutorial.h"
#include "gt/gfx/draw_queue.h"
#include "gt/input.h"
#include "gt/audio/music.h"
#include "gen/assets/bg/buttons.json.h"
#include "gen/assets/music.h"
char tutorial_step;
char tutorial_frame;
char tutorial_subframe;
char tutorial_score;
SpriteSlot tutorialGfx;


#define TUTORIAL_Y 80

extern char puzzle_counter_ones;


void init_tutorial(SpriteSlot gfx) {
    tutorial_step = TUTORIAL_STEP_MOVE;
    tutorialGfx = gfx;
    tutorial_frame = 0;
    tutorial_subframe = 0;
    tutorial_score = 0;
}

void draw_tutorial() {
    ++tutorial_subframe;
    if(tutorial_subframe == 8) {
        tutorial_subframe = 0;
        ++tutorial_frame;
        tutorial_frame &= 3;
    }
    switch(tutorial_step) {
        case TUTORIAL_STEP_MOVE:
            queue_draw_sprite_frame(tutorialGfx, 28, TUTORIAL_Y, tutorial_frame + BUTTONS_TAG_SIDES_START, 0);
            queue_draw_sprite_frame(tutorialGfx, 52, TUTORIAL_Y, BUTTONS_TAG_MOVE_START, 0);
            break;
        case TUTORIAL_STEP_SPIN:
            queue_draw_sprite_frame(tutorialGfx, 28, TUTORIAL_Y, tutorial_frame + BUTTONS_TAG_AB_START, 0);
            queue_draw_sprite_frame(tutorialGfx, 52, TUTORIAL_Y, BUTTONS_TAG_SPIN_START, 0);
            break;
        case TUTORIAL_STEP_SHOOT:
            queue_draw_sprite_frame(tutorialGfx, 28, TUTORIAL_Y, tutorial_frame + BUTTONS_TAG_C_START, 0);
            queue_draw_sprite_frame(tutorialGfx, 52, TUTORIAL_Y, BUTTONS_TAG_SHOOT_START, 0);
            break;
        case TUTORIAL_STEP_BOSS:
            queue_draw_sprite_frame(tutorialGfx, 28, TUTORIAL_Y, BUTTONS_TAG_RED_START, 0);
            queue_draw_sprite_frame(tutorialGfx, 52, TUTORIAL_Y, BUTTONS_TAG_BOSS_START, 0);
            break;
        default:
            break;
    }
}

void check_tutorial_conditions() {
    static char did_advance;
    did_advance = 0;
    switch(tutorial_step) {
        case TUTORIAL_STEP_MOVE:
            if(player1_new_buttons & (INPUT_MASK_LEFT | INPUT_MASK_RIGHT)) {
                ++tutorial_score;
                if(tutorial_score == 3) {
                    did_advance = 1;
                }
            }
            break;
        case TUTORIAL_STEP_SPIN:
            if(player1_new_buttons & (INPUT_MASK_A | INPUT_MASK_B)) {
                ++tutorial_score;
                if(tutorial_score == 3) {
                    did_advance = 1;
                }
            }
            break;
        case TUTORIAL_STEP_SHOOT:
            if(puzzle_counter_ones == 1) {
                did_advance = 1;
            }
            break;
        case TUTORIAL_STEP_BOSS:
            break;
        default:
            break;
    }
    if(did_advance) {
        ++tutorial_step;
        play_sound_effect(ASSET__music__correct_sfx_ID, 1);
        tutorial_score = 0;
    }
}