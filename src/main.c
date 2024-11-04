#include "raylib.h"
#include <stdio.h>

typedef struct {
    int x;
    int y;
} iVec2D;

typedef enum {
    FOOD_CHERRY,
    FOOD_SPRITE_COUNT
} FoodSpriteIdx;

typedef enum {
    BACKGROUND_BLACK_BORDER,
    BACKGROUND_DIRT,
    BACKGROUND_SPRITE_COUNT
} BackgroundSpriteIdx;

typedef enum  {
    SNAKE_HEAD_H,
    SNAKE_HEAD_V,
    SNAKE_BODY_H,
    SNAKE_BODY_V,
    SNAKE_BODY_L_U,
    SNAKE_BODY_R_U,
    SNAKE_BODY_R_D,
    SNAKE_BODY_L_D,
    SNAKE_TAIL_H,
    SNAKE_TAIL_V,
    SNAKE_SPRITE_COUNT
} SnakeSpriteIdx;

int main(char** argv, int argc) {
    //TODO:: parse cmd line to get screen w/h and vsync/framerate
    const iVec2D screen_size = {800,800};
    const iVec2D grid_size = {8,8};
    const int grid_width = 16;
    

    InitWindow(screen_size.x,screen_size.y, "raylib basic window");
    SetTargetFPS(60);

    // sprite stuff
    Texture2D snake_sprites = LoadTexture("assets/snake_spritesheet.bmp");
    Texture2D food_sprites = LoadTexture("assets/food_spritesheet.bmp");
    Texture2D background_sprites = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    Rectangle sprite_rect = {0, 0, grid_width, grid_width};
    Rectangle background_rect = {BACKGROUND_DIRT * grid_width, 0, grid_width, grid_width};
    Vector2 snake_pos = {0, 0};
    Vector2 food_pos = {grid_width, 0};
    Vector2 background_pos = {grid_width*2, 0};
    

    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)screen_size.x / (grid_size.x * grid_width) ; //base on screen width only right now.

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        //background fill
        for (int x = 0; x < grid_size.x; x++) {
            for (int y = 0; y < grid_size.y; y++) {
                Vector2 pos = {x * grid_width, y * grid_width};
                DrawTextureRec(background_sprites, background_rect, pos, WHITE);
            }
        }
        DrawTextureRec(snake_sprites, sprite_rect, snake_pos, WHITE);
        DrawTextureRec(food_sprites, sprite_rect, food_pos, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;

}

