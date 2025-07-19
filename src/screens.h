
#ifndef SNAKE_SCREENS_H
#define SNAKE_SCREENS_H

typedef struct GameState GameState;

//TODO:: MOVE TO SCREENS
typedef enum {
    SPRITESHEET_SNAKE,
    SPRITESHEET_FOOD,
    SPRITESHEET_BACKGROUND,
    SPRITESHEET_COUNT
} SpriteSheet;

//Menu screen
void setup_menuscreen(GameState* state);
void update_menuscreen(GameState* state);
void draw_menuscreen(GameState* state);
void unload_menuscreen(GameState* state);

//Game screen
void setup_gamescreen(GameState* state);
void update_gamescreen(GameState* state);
void draw_gamescreen(GameState* state);
void unload_gamescreen(GameState* state);

#endif