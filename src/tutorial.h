#include "gt/gfx/sprites.h"
#define TUTORIAL_STEP_MOVE 0
#define TUTORIAL_STEP_SPIN 1
#define TUTORIAL_STEP_SHOOT 2
#define TUTORIAL_STEP_BOSS 3
#define TUTORIAL_DONE 4

void init_tutorial(SpriteSlot gfx);

void draw_tutorial();

void check_tutorial_conditions();