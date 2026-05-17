
#ifndef SNAKE_SCREENS_H
#define SNAKE_SCREENS_H

#include <stdbool.h>
#define SQUARE_PIXEL_WIDTH 16

typedef struct {
    int x;
    int y;
} iVec2D;

typedef enum {
    FOOD_CHERRY,
    FOOD_SPRITE_COUNT
} FoodSpriteIdx;

typedef enum {
    BACKGROUND_WHITETILE,
    BACKGROUND_DIRT,
    BACKGROUND_COUNT
} BackgroundSprite;
static const char* BACKGROUNDS_STRINGS[] = {"TILE", "DIRT"};

typedef struct PersistentSceneData {
    int game_fps;
    iVec2D screen_size;
    iVec2D grid_size;
    BackgroundSprite selected_background;
} PersistentSceneData;

typedef enum {
    SCENE_NONE,
    SCENE_GAME,
    SCENE_MENU,
    SCENE_COUNT
} Scene;

typedef enum {
    SCENE_FLAG_SCENECHANGE,
    SCENE_FLAG_COUNT
} SceneFlag;

typedef struct SceneState{
    PersistentSceneData* persist_data;
    void* screen_memory;

    Scene current_scene;
    Scene next_scene;

    bool flags[SCENE_FLAG_COUNT];
} SceneState;

//Implemented in menu_screen.c, should be in a sprites utility file
typedef struct Rectangle Rectangle;
Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y);

//Menu screen
void setup_menuscreen(SceneState* state);
void update_menuscreen(SceneState* state);
void draw_menuscreen(SceneState* state);
void unload_menuscreen(SceneState* state);

//Game screen
void setup_gamescreen(SceneState* state);
void update_gamescreen(SceneState* state);
void draw_gamescreen(SceneState* state);
void unload_gamescreen(SceneState* state);

#endif