#include "mode_menu.h"
#include "gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/input.h"
#include "gt/audio/music.h"

#include "gen/assets/music.h"

#define MENU_ITEM_COUNT 3

static SpriteSlot menu_gfx;
static char water_offset;
static char head_bob;
static char selected_item;
static char i;
static char tmpgy, tmpw;

char game_mode = 0;

static const char menu_item_gy[MENU_ITEM_COUNT] = {68, 88, 108};
static const char menu_item_w[MENU_ITEM_COUNT] = {68, 88, 108};
static const char menu_item_y[MENU_ITEM_COUNT] = {68, 84, 100};

static signed char menu_item_x[MENU_ITEM_COUNT];
static char menu_item_vx[MENU_ITEM_COUNT];

char do_mode_menu(SpriteSlot _menu_gfx) {
    menu_gfx = _menu_gfx;
    water_offset = 0;
    selected_item = 0;

    for(i = 0; i < MENU_ITEM_COUNT; ++i) {
        menu_item_x[i] = -16;
        menu_item_vx[i] = 0;
    }

    while(1) {
        await_vsync(1);
        flip_pages();
        update_inputs();
        tick_music();
        queue_draw_sprite(0, 0, 60, 64, 0, 0, menu_gfx);
        queue_draw_sprite(60, 0, 68, 64, 60, (head_bob >> 5) & 1, menu_gfx);
        tmpw = water_offset >> 1;
        queue_draw_tiled(0, 64, 127, 16, 112+tmpw, 64, menu_gfx);
        rect.y = 80;
        rect.gy = 80;
        rect.b = menu_gfx;
        queue_draw_tiled_rect();
        rect.y = 96;
        rect.gy = 96;
        rect.b = menu_gfx;
        queue_draw_tiled_rect();
        rect.y = 112;
        rect.gy = 112;
        rect.b = menu_gfx;
        queue_draw_tiled_rect();
        ++water_offset;
        water_offset &= 31;
        ++head_bob;
        for(i = 0; i < MENU_ITEM_COUNT; ++i) {
            tmpgy = menu_item_gy[i];
            tmpw = menu_item_w[i];
            if(selected_item == i) {
                tmpgy += 10;
                tmpw += 4;
                if(menu_item_x[i] > 0) menu_item_vx[i] = -1;
                else if(menu_item_x[i] < 0) menu_item_vx[i] = 1;
                else menu_item_vx[i] = 0;
            } else {
                if(menu_item_x[i] > -8) menu_item_vx[i] = -1;
                else if(menu_item_x[i] < -8) menu_item_vx[i] = 1;
                else menu_item_vx[i] = 0;
            }
            menu_item_x[i] += menu_item_vx[i];
            queue_draw_sprite(menu_item_x[i], menu_item_y[i], tmpw, 10, 0, tmpgy, menu_gfx);
        }
        queue_clear_border(32);
        if(player1_new_buttons & INPUT_MASK_DOWN) {
            ++selected_item;
            play_sound_effect(ASSET__music__shoot_sfx_ID, 2);
        }
        if(player1_new_buttons & INPUT_MASK_UP) {
            --selected_item;
            play_sound_effect(ASSET__music__shoot_sfx_ID, 2);
        }
        if(selected_item == 3) {
            selected_item = 0;
        } else if(selected_item > 2) {
            selected_item = 2;
        }
        if(player1_new_buttons & (INPUT_MASK_A | INPUT_MASK_START)) {
            game_mode = selected_item;
            break;
        }
        if(player1_new_buttons & INPUT_MASK_B) {
            return 0;
        }
        await_draw_queue();
    }
    await_vsync(1);
    flip_pages();
    return 1;
}