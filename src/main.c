#include "raylib.h"
#include <stdio.h> //printf
#include <stdlib.h> //malloc
#include <math.h> //fabsf, abs

typedef Vector2 fVec2D;

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

// typedef struct {
//     Texture2D sprite_sheet;
//     Rectangle sprite_rect;
//     Vector2 screen_position;
// } SpriteDrawData;

typedef enum Direction {
    DIR_NULL,
    DIR_UP,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT
} Direction;

typedef enum {
    SPRITE_SNAKE_IDX,
    SPRITE_FOOD_IDX,
    SPRITE_BACKGROUND_IDX,
    SPRITE_SHEET_COUNT
} SpriteSheetIdx;

typedef struct GameState{
    Direction* dir_queue;
    Texture2D* textures;
} GameState;

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
const int GAME_FPS = 60;
const float SNAKE_WAIT_FRAMES = 10.0f;
const float SNAKE_WAIT_TIME = SNAKE_WAIT_FRAMES / GAME_FPS;
const int KEY_QUEUE_LENGTH = 2;

fVec2D GridToPixelCoords(int grid_x, int grid_y) {
    fVec2D coord = {grid_x * SQUARE_PIXEL_WIDTH, grid_y * SQUARE_PIXEL_WIDTH};
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

//If it's valid, updates snake's facing direction and moves by one space
void move_snake(Snake* snake, Direction* direction_queue) {
    for (int i = 0; i < KEY_QUEUE_LENGTH; i++) {
        if (direction_queue[i] != DIR_NULL) {
            if (direction_queue[i] != flip_direction(snake->facing)) {
                snake->facing = direction_queue[i];
            }
            direction_queue[i] = DIR_NULL;
            break;
        }
    }
    
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
Rectangle screen_to_dest(fVec2D screen_position, Rectangle texture_rectangle) {
    float half_width = (float)SQUARE_PIXEL_WIDTH/2;
    Rectangle dest = {screen_position.x + half_width, screen_position.y + half_width, fabsf(texture_rectangle.width), fabsf(texture_rectangle.height)};
    return dest;
}

//Pick random unoccupied location to spawn the cherry.
//If there's an empty spot, cherry_location is populated and function returns true
//If there are no empty locations, returns false
bool spawn_cherry(Snake* snake, iVec2D* cherry_location) {
    //Set up bool array with snake locations true, all else false
    const int arr_len = GRID_SIZE.x * GRID_SIZE.y;
    static bool* presence_array = NULL;
    if (!presence_array) {presence_array = malloc(sizeof(bool) * arr_len);}
    for (int i = 0; i < arr_len; i++) { presence_array[i] = false; }
    
    for (int i = 0; i < snake->length; i++) {
        int idx = snake->nodes[i].x + snake->nodes[i].y * GRID_SIZE.x;
        presence_array[idx] = true;
    }

    //start with random spot and scan through array until empty place found or everything scanned.
    int choice = GetRandomValue(0, arr_len - 1);
    int count = 1;
    while (presence_array[choice] && count > arr_len) {
        choice++;
        count++;
        if (choice >= arr_len) {choice = 0;}
    }

    //If it's empty, populate coordinates. Otherwise return false.
    if (!presence_array[choice]) {
        cherry_location->x = choice % GRID_SIZE.x;
        cherry_location->y = choice / GRID_SIZE.x;
        return true;
    }
    return false;
}

//Draws the snake to the screen
//TODO:: put drawing logic for head/tail in loop as well?
void draw_snake(Snake* snake, Texture2D sprite_sheet) {
    Direction direction;
    fVec2D screen_pos;
    fVec2D origin = {(float)SQUARE_PIXEL_WIDTH/2, (float)SQUARE_PIXEL_WIDTH/2};
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
                sprite_index = SNAKE_BODY_R_D;
                break;
            case (DIR_UP<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_V;
                break;
            case (DIR_UP<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_L_D;
                break;
            case (DIR_RIGHT<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_L_U;
                break;
            case (DIR_RIGHT<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_L_D;
                break;
            case (DIR_RIGHT<<4) + DIR_RIGHT:
                sprite_index = SNAKE_BODY_H;
                break;
            case (DIR_DOWN<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_V;
                break;
            case (DIR_DOWN<<4) + DIR_RIGHT:
                sprite_index = SNAKE_BODY_R_U;
                break;
            case (DIR_DOWN<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_L_U;
                break;
            case (DIR_LEFT<<4) + DIR_UP:
                sprite_index = SNAKE_BODY_R_U;
                break;
            case (DIR_LEFT<<4) + DIR_LEFT:
                sprite_index = SNAKE_BODY_H;
                break;
            case (DIR_LEFT<<4) + DIR_DOWN:
                sprite_index = SNAKE_BODY_R_D;
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

void squish_key_queue(Direction* queue){
    //Set s to first NULL in queue
    Direction *s = queue;
    Direction *e = queue + KEY_QUEUE_LENGTH;
    while (s < e && *s != DIR_NULL) {s++;}
    
    //Swap non-null values into s until no more found
    Direction *c = s + 1;
    while (c < e) {
        if (*c != DIR_NULL) {
            *s = *c;
            *c = DIR_NULL;
            s++;
        }
        c++; //ha
    }
}

void handle_input(GameState* state) {
    //Compactify queue
    squish_key_queue(state->dir_queue);
    Direction *d = state->dir_queue;
    Direction *e = state->dir_queue + KEY_QUEUE_LENGTH;
    while (d < e && *d != DIR_NULL) {d++;}

    KeyboardKey key;
    while (key = GetKeyPressed()) {
        if (d == e) {continue;}
        switch(key) {
            case KEY_RIGHT:
                *d = DIR_RIGHT;
                break;
            case KEY_LEFT:
                *d = DIR_LEFT;
                break;
            case KEY_UP:
                *d = DIR_UP;
                break;
            case KEY_DOWN:
                *d = DIR_DOWN;
                break;
        }
        d++;
    }
}


void draw_food(iVec2D food_pos, Texture2D sprite_sheet) {
    fVec2D screen_pos = GridToPixelCoords(food_pos.x, food_pos.y);
    Rectangle sprite_rect = GetSpriteRect(FOOD_CHERRY, SQUARE_PIXEL_WIDTH);
    DrawTextureRec(sprite_sheet, sprite_rect, screen_pos, WHITE);
}

int main(char** argv, int argc) {
    //TODO:: parse cmd line to get screen w/h and vsync/framerate

    InitWindow(SCREEN_SIZE.x,SCREEN_SIZE.y, "c-snake");
    SetTargetFPS(GAME_FPS);
    
    //init game state. TODO:: move to separate function
    GameState state;
    state.textures = malloc(sizeof(Texture2D) * SPRITE_SHEET_COUNT);
    state.textures[SPRITE_FOOD_IDX] = LoadTexture("assets/food_spritesheet.bmp");
    state.textures[SPRITE_SNAKE_IDX] = LoadTexture("assets/snake_spritesheet.bmp");
    state.textures[SPRITE_BACKGROUND_IDX] = LoadTexture("assets/backgrounds_spritesheet.bmp");
    state.dir_queue = malloc(sizeof(Direction) * KEY_QUEUE_LENGTH);
    for (int i = 0; i < KEY_QUEUE_LENGTH; i++) {state.dir_queue[i] = DIR_NULL;}
    state.dir_queue[0] = DIR_UP;
    Rectangle background_rect = GetSpriteRect(BACKGROUND_BLACK_BORDER, SQUARE_PIXEL_WIDTH);
    iVec2D food_pos;
    iVec2D snake_start =  {2,2};
    Snake* snake = make_snake(snake_start, state.dir_queue[0], 8);
    spawn_cherry(snake, &food_pos);

    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)SCREEN_SIZE.x / (GRID_SIZE.x * SQUARE_PIXEL_WIDTH) ; //base on screen width only right now.

    double time = GetTime();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        handle_input(&state);
        
        //Draw background
        //TODO::draw background once to a texture and use that for rest of game
        for (int x = 0; x < GRID_SIZE.x; x++) {
            for (int y = 0; y < GRID_SIZE.y; y++) {
                Vector2 pos = {x * SQUARE_PIXEL_WIDTH, y * SQUARE_PIXEL_WIDTH};
                DrawTextureRec(state.textures[SPRITE_BACKGROUND_IDX], background_rect, pos, WHITE);
            }
        }
        
        draw_food(food_pos, state.textures[SPRITE_FOOD_IDX]);
        draw_snake(snake, state.textures[SPRITE_SNAKE_IDX]);
        
        if (GetTime() - time  > SNAKE_WAIT_TIME) {
            move_snake(snake, state.dir_queue);
            time = GetTime();
        }
        
        EndDrawing();
    }
    CloseWindow();
    destroy_snake(snake);

    return 0;
}

