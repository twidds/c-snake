#include "raylib.h"
#include <stdio.h> //printf
#include <stdlib.h> //malloc
#include <math.h> //fabsf, abs


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

typedef enum Direction {
    DIR_UP,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT
} Direction;


// typedef struct Node {
//     Point2D position;
// } Node;

typedef struct Snake {
    // Direction buffered_dir;
    // Direction move_dir;
    Direction facing;
    iVec2D* nodes; //points at tail
    int length;
    int max_length;
} Snake;


const iVec2D SCREEN_SIZE = {800,800};
const iVec2D GRID_SIZE = {20,20};
const int SQUARE_PIXEL_WIDTH = 16; //must match width of sprites


Vector2 GridToPixelCoords(int grid_x, int grid_y) {
    Vector2 coord = {grid_x * SQUARE_PIXEL_WIDTH, grid_y * SQUARE_PIXEL_WIDTH};
    return coord;
}

Rectangle GetSpriteRect(int sprite_index, int sprite_width) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    return rect;
}



//If a coordinate is outside the bounds of the grid, this will wrap it back into bounds.
void wrap_coordinate(iVec2D* coordinate) {
    coordinate->x = coordinate->x % GRID_SIZE.x;
    coordinate->y = coordinate->y % GRID_SIZE.y;
    if (coordinate->x < 0) {coordinate->x += GRID_SIZE.x;}
    if (coordinate->y < 0) {coordinate->y += GRID_SIZE.y;}
}

//Returns the coordinate distance away from the start in the direction specified.
//Will wrap the coordinate based on grid sizes
iVec2D get_coord_offset(iVec2D start, Direction direction, int distance) {
    iVec2D coord = start;
    switch (direction) {
        case DIR_UP:
            coord.y -= 1;
            break;
        case DIR_RIGHT:
            coord.x += 1;
            break;
        case DIR_DOWN:
            coord.y += 1;
            break;
        case DIR_LEFT:
            coord.x -= 1;
            break;
    }
    wrap_coordinate(&coord);
    return coord;
}

void norm_iVec2D(iVec2D* vec) {
    vec->x = vec->x != 0 ? vec->x / abs(vec->x) : vec->x;
    vec->y = vec->y != 0 ? vec->y / abs(vec->y) : vec->y;
}

Direction flip_direction(Direction d) {
    switch (d) {
        case DIR_UP: return DIR_DOWN;
        case DIR_RIGHT: return DIR_LEFT;
        case DIR_DOWN: return DIR_UP;
        case DIR_LEFT: return DIR_RIGHT;
    }
}

//Moves the snake forward by one
void move_snake(Snake* snake) {
    for (int i = 0; i < snake->length - 1; i++) {
        snake->nodes[i] = snake->nodes[i+1];
    }
    iVec2D* head = &snake->nodes[snake->length - 1];
    iVec2D new_head = get_coord_offset(*head, snake->facing, 1);
    *head = new_head;
}

//Makes a straight snake, with tail at the specified coordinate and facing the direction indicated
//Length less than 2 is not valid and will return null
Snake* make_snake(iVec2D tail_coord, Direction direction, int length) {
    if (length < 2) {
        return NULL;
    }

    Snake* snake = malloc(sizeof(Snake));
    snake->facing = direction;
    snake->length = length;
    snake->max_length = length;
    snake->nodes = malloc(sizeof(iVec2D) * length);
    
    for (int i = 0; i < length; i++) {
        snake->nodes[i] = tail_coord;
        tail_coord = get_coord_offset(tail_coord, direction, 1);
    }
    return snake;
}

void destroy_snake(Snake* snake) {
    free(snake->nodes);
    free(snake);
}


//Returns the direction to get from start to end assuming they are only offset by X OR Y
//Takes grid wrapping into account
//If coordinate is diagonal then this is undefined
void get_coord_direction(iVec2D start, iVec2D end) {

}

//subtracts result of (first - second)
iVec2D sub_iVec2D(iVec2D first, iVec2D second) {
    iVec2D result;
    result.x = first.x - second.x;
    result.y = first.y - second.y;
    return result;
}


Direction get_snake_node_direction(iVec2D start_node, iVec2D end_node) {
    iVec2D sub = sub_iVec2D(end_node, start_node);
    bool wrap = sub.x < -1 || sub.x > 1 || sub.y < -1 || sub.y > 1;
    Direction result;
    if (sub.x != 0) {
        if (sub.x < 0) {
            result = DIR_LEFT;
        } else {
            result = DIR_RIGHT;
        }
    } else {
        if (sub.y < 0) {
            result = DIR_UP;
        } else {
            result = DIR_DOWN;
        }
    }
    if (wrap) {
        result = flip_direction(result);
    }
    return result;
}

//Translates screen position and texture rectangle to a destination rectangle.
//Note that it also offsets the screen position to center the texture (makes rotation easier later)
Rectangle screen_to_dest(Vector2 screen_position, Rectangle texture_rectangle) {
    float half_width = (float)SQUARE_PIXEL_WIDTH/2;
    Rectangle dest = {screen_position.x + half_width, screen_position.y + half_width, fabsf(texture_rectangle.width), fabsf(texture_rectangle.height)};
    return dest;
}

//Draws the snake to the screen
//TODO:: put drawing logic for head/tail in loop as well?
void draw_snake(Snake* snake, Texture2D sprite_sheet) {
    Direction direction;
    Vector2 screen_pos;
    Vector2 origin = {(float)SQUARE_PIXEL_WIDTH/2, (float)SQUARE_PIXEL_WIDTH/2};
    // Vector2 origin = {0.0f, 0.0f};
    float rotation;
    SnakeSpriteIdx sprite_index;
    Rectangle sprite_rect;
    Rectangle destination;
    
    //Draw tail
    iVec2D tail = snake->nodes[0];
    iVec2D next = snake->nodes[1];
    direction = get_snake_node_direction(tail, next);
    screen_pos = GridToPixelCoords(tail.x, tail.y);
    switch (direction) {
        case DIR_UP:
            sprite_index = SNAKE_TAIL_V;
            rotation = 0.0f;
            break;
        case DIR_RIGHT:
            sprite_index = SNAKE_TAIL_H;
            rotation = 180.0f;
            break;
        case DIR_DOWN:
            sprite_index = SNAKE_TAIL_V;
            rotation = 180.0f;
            break;
        case DIR_LEFT:
            sprite_index = SNAKE_TAIL_H;
            rotation = 0.0f;
            break;
    }
    sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH);
    // DrawTextureRec(sprite_sheet, sprite_rect, screen_pos, WHITE);
    DrawTexturePro(sprite_sheet, sprite_rect, screen_to_dest(screen_pos, sprite_rect), origin, rotation, WHITE);

    //Draw head
    iVec2D head = snake->nodes[snake->length - 1];
    iVec2D prev = snake->nodes[snake->length - 2];
    direction = get_snake_node_direction(prev, head);
    screen_pos = GridToPixelCoords(head.x, head.y);
    switch (direction) {
        case DIR_UP:
            sprite_index = SNAKE_HEAD_V;
            rotation = 0.0f;
            break;
        case DIR_RIGHT:
            sprite_index = SNAKE_HEAD_H;
            rotation = 0.0f;
            break;
        case DIR_DOWN:
            sprite_index = SNAKE_HEAD_V;
            rotation = 180.0f;
            break;
        case DIR_LEFT:
            sprite_index = SNAKE_HEAD_H;
            rotation = 180.0f;
            break;
    }
    sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH);
    DrawTexturePro(sprite_sheet, sprite_rect, screen_to_dest(screen_pos, sprite_rect), origin, rotation, WHITE);


    //Draw body
    rotation = 0.0f;
    for (int i = 1; i < snake->length - 1; i++) {
        iVec2D part = snake->nodes[i];
        prev = snake->nodes[i-1];
        next = snake->nodes[i+1];
        Direction prev_dir = get_snake_node_direction(prev, part);
        Direction next_dir = get_snake_node_direction(part, next);
        screen_pos = GridToPixelCoords(part.x, part.y);
        switch ((prev_dir<<4) + next_dir) {
            case (DIR_UP<<4) + DIR_RIGHT:
                sprite_index = SNAKE_BODY_R_U;
                break;
            case (DIR_UP<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_V;
                break;
            case (DIR_UP<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_L_U;
                break;
            case (DIR_RIGHT<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_R_U;
                break;
            case (DIR_RIGHT<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_R_D;
                break;
            case (DIR_RIGHT<<4) + DIR_RIGHT:
                sprite_index = SNAKE_BODY_H;
                break;
            case (DIR_DOWN<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_V;
                break;
            case (DIR_DOWN<<4) + DIR_RIGHT:
                sprite_index = SNAKE_BODY_R_D;
                break;
            case (DIR_DOWN<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_L_D;
                break;
            case (DIR_LEFT<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_L_U;
                break;
            case (DIR_LEFT<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_H;
                break;
            case (DIR_LEFT<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_L_D;
                break;
        }
        
        sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH);
        DrawTexturePro(sprite_sheet, sprite_rect, screen_to_dest(screen_pos, sprite_rect), origin, rotation, WHITE);
    }

}

//Grows head by one in the direction it's facing
//TODO:: implement
void grow_snake(Snake* snake) {

}

//Test snake draw data generator
// SpriteDrawData* GetSnakeData(Texture2D sprite_sheet) {
//     const int num = 9;
//     SpriteDrawData *data = malloc(sizeof(SpriteDrawData)*num);
//     //HEAD
//     data[0].screen_position = GridToPixelCoords(0,0);
//     data[0].sprite_rect = GetSpriteRect(SNAKE_HEAD_V, SQUARE_PIXEL_WIDTH);
//     data[0].sprite_sheet = sprite_sheet;

//     data[1].screen_position = GridToPixelCoords(0,1);
//     data[1].sprite_rect = GetSpriteRect(SNAKE_BODY_V, SQUARE_PIXEL_WIDTH);
//     data[1].sprite_sheet = sprite_sheet;

//     data[2].screen_position = GridToPixelCoords(0,2);
//     data[2].sprite_rect = GetSpriteRect(SNAKE_BODY_R_U, SQUARE_PIXEL_WIDTH);
//     data[2].sprite_sheet = sprite_sheet;

//     data[3].screen_position = GridToPixelCoords(1,2);
//     data[3].sprite_rect = GetSpriteRect(SNAKE_BODY_H, SQUARE_PIXEL_WIDTH);
//     data[3].sprite_sheet = sprite_sheet;

//     data[4].screen_position = GridToPixelCoords(2,2);
//     data[4].sprite_rect = GetSpriteRect(SNAKE_BODY_L_D, SQUARE_PIXEL_WIDTH);
//     data[4].sprite_sheet = sprite_sheet;

//     data[5].screen_position = GridToPixelCoords(2,3);
//     data[5].sprite_rect = GetSpriteRect(SNAKE_BODY_L_U, SQUARE_PIXEL_WIDTH);
//     data[5].sprite_sheet = sprite_sheet;

//     data[6].screen_position = GridToPixelCoords(1,3);
//     data[6].sprite_rect = GetSpriteRect(SNAKE_BODY_H, SQUARE_PIXEL_WIDTH);
//     data[6].sprite_sheet = sprite_sheet;

//     data[7].screen_position = GridToPixelCoords(0,3);
//     data[7].sprite_rect = GetSpriteRect(SNAKE_BODY_R_D, SQUARE_PIXEL_WIDTH);
//     data[7].sprite_sheet = sprite_sheet;

//     data[8].screen_position = GridToPixelCoords(0,4);
//     data[8].sprite_rect = GetSpriteRect(SNAKE_TAIL_V, SQUARE_PIXEL_WIDTH);
//     data[8].sprite_sheet = sprite_sheet;

//     return data;
// }

int main(char** argv, int argc) {
    //TODO:: parse cmd line to get screen w/h and vsync/framerate

    InitWindow(SCREEN_SIZE.x,SCREEN_SIZE.y, "raylib basic window");
    SetTargetFPS(60);

    // sprite stuff
    Texture2D snake_sprites = LoadTexture("assets/snake_spritesheet.bmp");
    Texture2D food_sprites = LoadTexture("assets/food_spritesheet.bmp");
    Texture2D background_sprites = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    Rectangle background_rect = GetSpriteRect(BACKGROUND_BLACK_BORDER, SQUARE_PIXEL_WIDTH);

    SpriteDrawData food;
    food.screen_position = GridToPixelCoords(4,2);
    food.sprite_rect = GetSpriteRect(FOOD_CHERRY, SQUARE_PIXEL_WIDTH);
    food.sprite_sheet = food_sprites;

    // SpriteDrawData* snake = GetSnakeData(snake_sprites);
    
    iVec2D snake_start =  {2,2};
    Snake* snake = make_snake(snake_start, DIR_UP, 2);
    for (int i = 0; i < snake->length; i++) {
        printf("%d,%d\r\n", snake->nodes[i].x, snake->nodes[i].y);
    }

    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)SCREEN_SIZE.x / (GRID_SIZE.x * SQUARE_PIXEL_WIDTH) ; //base on screen width only right now.

    double time = GetTime();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        
        //Draw background
        for (int x = 0; x < GRID_SIZE.x; x++) {
            for (int y = 0; y < GRID_SIZE.y; y++) {
                Vector2 pos = {x * SQUARE_PIXEL_WIDTH, y * SQUARE_PIXEL_WIDTH};
                DrawTextureRec(background_sprites, background_rect, pos, WHITE);
            }
        }
        
        //Draw Food
        DrawTextureRec(food.sprite_sheet, food.sprite_rect, food.screen_position, WHITE);

        //Draw snek
        draw_snake(snake, snake_sprites);
        
        if (GetTime() - time  > 0.2) {
            move_snake(snake);
            time = GetTime();
        }
        
        // for (int i = 0; i < 9; i++) {
        //     DrawTextureRec(snake[i].sprite_sheet, snake[i].sprite_rect, snake[i].screen_position, WHITE);
        // }
        EndDrawing();
    }
    CloseWindow();

    destroy_snake(snake);
    return 0;

}

