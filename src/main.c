#include "gt/gametank.h"
#include "gt/gfx/draw_queue.h"
#include "gt/audio/music.h"
#include "gen/assets/music.h"
#include "gen/assets/bg.h"

#include "gt/gfx/sprites.h"

SpriteSlot bgImg;

int main () {
 
    init_graphics();
    bgImg = allocate_sprite(&ASSET__bg__scene_bmp_load_list);

    play_song(ASSET__music__katachidasu_mid, REPEAT_LOOP);

    while (1) {                                     //  Run forever
        //queue_clear_screen(3);
        queue_draw_sprite(0, 0, 127, 127, 0, 0, bgImg);

        queue_clear_border(0);

        await_draw_queue();
        await_vsync(1);
        flip_pages();
        tick_music();
    }
 
  return (0);                                     //  We should never get here!
}