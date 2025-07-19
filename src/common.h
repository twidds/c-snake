#ifndef SNAKE_COMMON_H
#define SNAKE_COMMON_H

#include <stdbool.h>

typedef struct {
    int x;
    int y;
} iVec2D;

typedef enum {
    SCENE_GAME,
    SCENE_MENU,
    SCENE_SETTINGS //unused
} Scene;

typedef enum {
    FLAG_SCENECHANGE,
    FLAG_GAME_PAUSED,
    FLAG_INVINCIBLE,
    FLAG_GAMEOVER,
    FLAG_GAMEWIN,
    FLAG_ATECHERRY,
    FLAG_BACKGROUNDCHANGE,
    FLAG_GAMERESET,
    FLAG_SHOWDEBUG,
    FLAGS_COUNT
} GameFlag;

typedef struct GameState{
    Scene current_scene;
    Scene next_scene;
    iVec2D screen_size;
    iVec2D grid_size;
    bool flags[FLAGS_COUNT];
    
    // DirectionQueue dir_queue;
    // Backgroundsprite background; //
    // double tick_frames;
    // double tick_time; //tracks time since last tick

    void* screen_memory;
    // Camera2D camera;
    // Texture2D* textures;
    // RenderTexture2D screen_target;
} GameState;

#endif