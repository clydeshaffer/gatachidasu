#include "gt/gfx/sprites.h"

#define GRID_CENTER_X 40
#define GRID_CENTER_Y_START 45

#define GRID_SPRITE_X 29
#define GRID_SPRITE_Y 42
#define GRID_SPRITE_CENTER_X 37
#define GRID_SIZE 5
#define GRID_FULL_COUNT 25
#define GRID_SQUARE_OFFSET -4
#define GRID_SQUARE_SIZE 8

#define GRID_DRAW_RESULT_NONE 0
#define GRID_DRAW_RESULT_LOSE 1
#define GRID_DRAW_RESULT_WIN 2
#define GRID_DRAW_RESULT_PRE_WIN 4

extern char grid_rotation;
extern char grid_y_pos;

void grid_init(SpriteSlot s);

#define GRID_RESET_PUZZLE ((unsigned char*)0xFFFF)
void grid_setup_puzzle(const unsigned char *shape);

char grid_send_bullet(char x);

char grid_draw();