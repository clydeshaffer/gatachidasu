#include "gt/gfx/sprites.h"

#define GRID_CENTER_X 40
#define GRID_CENTER_Y_START 45

#define GRID_SPRITE_X 0
#define GRID_SPRITE_Y 0
#define GRID_SPRITE_CENTER_X 16
#define GRID_SIZE 5
#define GRID_FULL_COUNT 25
#define GRID_SQUARE_OFFSET -4
#define GRID_SQUARE_SIZE 8

#define GRID_DRAW_RESULT_NONE 0
#define GRID_DRAW_RESULT_LOSE 1
#define GRID_DRAW_RESULT_WIN 2
#define GRID_DRAW_RESULT_PRE_WIN 4

#define BULLET_GX 32
#define BULLET_GY 0

#define TROLL_MOVE_NONE 0
#define TROLL_MOVE_ORBIT 1
#define TROLL_MOVE_VIBRATE 2
#define TROLL_MOVE_FALLING 3
#define TROLL_MOVE_REGROWTH 4
#define TROLL_MOVE_ORBIT_EASY 5
#define TROLL_MOVE_LOOSE 6

//hehehe this one is a bitmask >;)
#define TROLL_INFO_NONE 0
#define TROLL_INFO_FLASHCARD 1
#define TROLL_INFO_MIRROR 2
#define TROLL_INFO_INVERT 4

#define BOSS_CONFIG_COUNT 8

extern char troll_move_mode;
extern char troll_info_mode;

extern char grid_rotation;
extern signed char grid_angular_momentum;
extern char grid_y_pos;
extern char grid_time;

void grid_init(SpriteSlot s);

#define GRID_RESET_PUZZLE ((unsigned char*)0xFFFF)
void grid_setup_puzzle(const unsigned char *shape);

char grid_send_bullet(char x);

char grid_draw();

void setup_troll_modes(char bossnum);