//MUST INCLUDE windows.h BEFORE THIS HEADER

#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "input.h"
#include "graphics.h"
#include <stdbool.h>
#include <time.h>

typedef enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Node {
    Point2D position;
} Node;

typedef struct Snake {
    Direction buffered_dir;
    Direction move_dir;
    Node* nodes; //points at tail
    int length;
    int max_length;
} Snake;

typedef struct CmdConfig {
    int window_x;
    int window_y;
    int board_x;
    int board_y;
} CmdConfig;

typedef enum GameFlag {
    FLAG_RUNNING,
    FLAG_PAUSED,
    FLAG_PAUSING,
    FLAG_SHOW_STATS,
    FLAGS_COUNT
} GameFlag;

typedef enum GameStats {
    STAT_N_RENDERS,
    STAT_N_DRAWS,
    STAT_FPS,
    STAT_FRAME_MS,
    STAT_MSG_MS,
    STAT_PAINT_MS,
    STAT_RENDER_MS,
    STAT_COUNT
} GameStats;

typedef struct FpsTimer {
    int cur_fps;
    int cur_index;
    clock_t* frame_times;
    int n_trackedframes;
} FpsTimer;

typedef struct GameState {
    bool game_flags[FLAGS_COUNT];
    int game_stats[STAT_COUNT];
    HWND main_window;
    FpsTimer fps_timer;
    LARGE_INTEGER clock_freq;
    int score;

    Point2D window_size;
    Point2D board_size;
    Snake game_snake;
    Node apple;

    GraphicsData graphics_data;
} GameState;

extern const char* STAT_STRINGS[STAT_COUNT];

HWND setup_window(const wchar_t* WindowName, const wchar_t* WindowText, void* AppData, int x_size, int y_size);

int parse_cmdline(CmdConfig* config, char** argv, int argc);
void realloc_snake_nodes(Snake* snake, int newsize);
void delete_fpstracker(FpsTimer* tracker);
void init_fpstracker(FpsTimer* tracker , int n_trackedframes);
void update_fpstracker(FpsTimer* tracker, clock_t latest);
void swap_buffers(void** bufa, void** bufb);
int setup_game(GameState* state, CmdConfig* config);
LRESULT game_exit(GameState* state);
void cleanup_game(GameState* state);
GpStatus render_game(GameState* state, HWND hwnd);
GpStatus render_pause (GameState* state, HWND hwnd);
LRESULT paint_window(HWND hwnd, GameState* state);
LRESULT handle_keys(HWND hwnd, GameState* state, WPARAM wParam, LPARAM lParam);
void run_game(GameState* state);

#endif /* SNAKE_GAME_H*/