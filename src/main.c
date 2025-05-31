#include "raylib.h"
#include <stdio.h> //printf
#include <stdlib.h> //malloc


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

typedef struct {
    Texture2D sprite_sheet;
    Rectangle sprite_rect;
    Vector2 screen_position;
} SpriteDrawData;


const iVec2D SCREEN_SIZE = {800,800};
const iVec2D GRID_SIZE = {8,8};
const int GRID_WIDTH = 16;


Vector2 GridToPixelCoords(int grid_x, int grid_y) {
    Vector2 coord = {grid_x * GRID_WIDTH, grid_y * GRID_WIDTH};
    return coord;
}

Rectangle GetSpriteRect(int sprite_index, int sprite_width) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    return rect;
}

//Test snake draw data generator
SpriteDrawData* GetSnakeData(Texture2D sprite_sheet) {
    const int num = 9;
    SpriteDrawData *data = malloc(sizeof(SpriteDrawData)*num);
    //HEAD
    data[0].screen_position = GridToPixelCoords(0,0);
    data[0].sprite_rect = GetSpriteRect(SNAKE_HEAD_V, GRID_WIDTH);
    data[0].sprite_sheet = sprite_sheet;

    data[1].screen_position = GridToPixelCoords(0,1);
    data[1].sprite_rect = GetSpriteRect(SNAKE_BODY_V, GRID_WIDTH);
    data[1].sprite_sheet = sprite_sheet;

    data[2].screen_position = GridToPixelCoords(0,2);
    data[2].sprite_rect = GetSpriteRect(SNAKE_BODY_R_U, GRID_WIDTH);
    data[2].sprite_sheet = sprite_sheet;

    data[3].screen_position = GridToPixelCoords(1,2);
    data[3].sprite_rect = GetSpriteRect(SNAKE_BODY_H, GRID_WIDTH);
    data[3].sprite_sheet = sprite_sheet;

    data[4].screen_position = GridToPixelCoords(2,2);
    data[4].sprite_rect = GetSpriteRect(SNAKE_BODY_L_D, GRID_WIDTH);
    data[4].sprite_sheet = sprite_sheet;

    data[5].screen_position = GridToPixelCoords(2,3);
    data[5].sprite_rect = GetSpriteRect(SNAKE_BODY_L_U, GRID_WIDTH);
    data[5].sprite_sheet = sprite_sheet;

    data[6].screen_position = GridToPixelCoords(1,3);
    data[6].sprite_rect = GetSpriteRect(SNAKE_BODY_H, GRID_WIDTH);
    data[6].sprite_sheet = sprite_sheet;

    data[7].screen_position = GridToPixelCoords(0,3);
    data[7].sprite_rect = GetSpriteRect(SNAKE_BODY_R_D, GRID_WIDTH);
    data[7].sprite_sheet = sprite_sheet;

    data[8].screen_position = GridToPixelCoords(0,4);
    data[8].sprite_rect = GetSpriteRect(SNAKE_TAIL_V, GRID_WIDTH);
    data[8].sprite_sheet = sprite_sheet;

    return data;
}

int main(char** argv, int argc) {
    //TODO:: parse cmd line to get screen w/h and vsync/framerate

    InitWindow(SCREEN_SIZE.x,SCREEN_SIZE.y, "raylib basic window");
    SetTargetFPS(60);

    // sprite stuff
    Texture2D snake_sprites = LoadTexture("assets/snake_spritesheet.bmp");
    Texture2D food_sprites = LoadTexture("assets/food_spritesheet.bmp");
    Texture2D background_sprites = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    Rectangle background_rect = GetSpriteRect(BACKGROUND_BLACK_BORDER, GRID_WIDTH);

    SpriteDrawData food;
    food.screen_position = GridToPixelCoords(4,2);
    food.sprite_rect = GetSpriteRect(FOOD_CHERRY, GRID_WIDTH);
    food.sprite_sheet = food_sprites;

    SpriteDrawData* snake = GetSnakeData(snake_sprites);

    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)SCREEN_SIZE.x / (GRID_SIZE.x * GRID_WIDTH) ; //base on screen width only right now.

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        
        //Draw background
        for (int x = 0; x < GRID_SIZE.x; x++) {
            for (int y = 0; y < GRID_SIZE.y; y++) {
                Vector2 pos = {x * GRID_WIDTH, y * GRID_WIDTH};
                DrawTextureRec(background_sprites, background_rect, pos, WHITE);
            }
        }
        
        //Draw Food
        DrawTextureRec(food.sprite_sheet, food.sprite_rect, food.screen_position, WHITE);

        //Draw snek
        for (int i = 0; i < 9; i++) {
            DrawTextureRec(snake[i].sprite_sheet, snake[i].sprite_rect, snake[i].screen_position, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();

    free(snake);
    return 0;

}

