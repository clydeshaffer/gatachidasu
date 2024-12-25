#include "gt/gfx/sprites.h"

#define GRID_CENTER_X 40
#define GRID_CENTER_Y_START 45

#define GRID_SPRITE_X 21
#define GRID_SPRITE_Y 26
#define GRID_SIZE 5
#define GRID_FULL_COUNT 25
#define GRID_SQUARE_OFFSET -4
#define GRID_SQUARE_SIZE 8

#define GRID_DRAW_RESULT_NONE 0
#define GRID_DRAW_RESULT_LOSE 1
#define GRID_DRAW_RESULT_WIN 2

extern char grid_rotation;
extern char grid_y_pos;

void grid_init(SpriteSlot s);

void grid_setup_puzzle(char* shape);

void grid_send_bullet(char x);

char grid_draw();