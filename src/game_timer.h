#define GAME_TIME_CAPPED(x) x.frames&128
#define GAME_TIME_SET_CAPPED(x) x.frames&=128

extern char game_timer_pos_x;
extern char game_timer_pos_y;
extern char puzzle_counter_pos_x;
extern char puzzle_counter_pos_y;

typedef struct game_time {
    unsigned char frames;
    unsigned char seconds_ones;
    unsigned char seconds_tens;
    unsigned char minutes_ones;
    unsigned char minutes_tens;
} game_time;

void init_game_timer_system();

void clear_game_timer();

void tick_game_timer();

char tick_puzzle_counter();

void render_game_timer();