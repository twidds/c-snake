#include "raylib.h"
#include <stdio.h> //snprintf
#include <stdlib.h> //malloc
#include <math.h> //fabsf, abs

typedef Vector2 fVec2D;

typedef struct {
    int x;
    int y;
} iVec2D;


const iVec2D SCREEN_SIZE = {800,800};
const iVec2D GRID_SIZE = {10,10};
const int SQUARE_PIXEL_WIDTH = 16; //must match width of sprites
const int GAME_FPS = 60;
const float MIN_TICK_WAIT_FRAMES = 0.0f;
const float MAX_TICK_WAIT_FRAMES = 60.0f;
const int KEY_QUEUE_LENGTH = 2;

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

typedef enum {
    FLAG_GAME_PAUSED,
    FLAG_INVINCIBLE,
    FLAG_GAMEOVER,
    FLAG_GAMEWIN,
    FLAG_ATECHERRY,
    FLAG_BACKGROUNDCHANGE,
    FLAG_GAMERESET,
    FLAG_SHOWDEBUG,
    FLAGS_COUNT
} GameFlagIdx;

typedef struct DirectionQueue{
    Direction* buf;
    int length;
    int max_length;
} DirectionQueue;

typedef struct GameState{
    double tick_frames;
    double tick_time; //tracks time since last tick
    DirectionQueue dir_queue;
    Texture2D* textures;
    BackgroundSpriteIdx background;
    bool* flags;
} GameState;



typedef struct Snake {
    Direction facing;
    iVec2D* nodes; //points at tail
    int length;
    int max_length;
} Snake;


void init_dirqueue(DirectionQueue* queue, int maxlength) {
    queue->length = 0;
    queue->max_length = maxlength;
    queue->buf = malloc(sizeof(Direction) * maxlength);
}

void pushb_dirqueue(DirectionQueue* queue, Direction dir) {
    if (queue->length < queue->max_length) {
        queue->buf[queue->length] = dir;
        queue->length += 1;
    } else {
        for (int i = 0; i < queue->length - 1; i++) {
            queue->buf[i] = queue->buf[i+1];
        }
        queue->buf[queue->length -1] = dir;
    }
}

Direction popf_dirqueue(DirectionQueue* queue) {
    if (queue->length > 0) {
        Direction dir = queue->buf[0];
        for (int i = 0; i < queue->length - 1; i++) {
            queue->buf[i] = queue->buf[i+1];
        }
        queue->length -= 1;
        return dir;
    }
    return DIR_NULL;
}

Direction peekf_dirqueue(DirectionQueue* queue) {
    if (queue->length > 0) {
        return queue->buf[0];
    }
    return DIR_NULL;
}

fVec2D GridToPixelCoords(int grid_x, int grid_y) {
    fVec2D coord = {grid_x * SQUARE_PIXEL_WIDTH, grid_y * SQUARE_PIXEL_WIDTH};
    return coord;
}

Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    rect.width = flip_x ? -1 * rect.width : rect.width;
    rect.height = flip_y ? -1 * rect.height : rect.height;
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

//Returns next queued key. This modifies the queue.
//If no keys are queued, returns DIR_NULL
Direction get_queued_direction(Direction* queue) {
    for (int i = 0; i < KEY_QUEUE_LENGTH; i++) {
        if (queue[i] != DIR_NULL) {
            Direction key = queue[i];
            queue[i] = DIR_NULL;
            return key;
        }
    }
    return DIR_NULL;
}

//If it's valid, updates snake's facing direction and moves by one space
void move_snake(Snake* snake, Direction queued_dir) {
    if (queued_dir != DIR_NULL && queued_dir != flip_direction(snake->facing)) {
        snake->facing = queued_dir;
    }

    for (int i = 0; i < snake->length - 1; i++) {
        snake->nodes[i] = snake->nodes[i+1];
    }
    iVec2D* head = &snake->nodes[snake->length - 1];
    iVec2D new_head = get_coord_offset(*head, snake->facing, 1);
    *head = new_head;
}

//Updates snake to be a straight line in direction provided starting at tail coordinate.
void set_snake(Snake* snake, iVec2D tail_coord, Direction direction, int length) {
    snake->facing = direction;
    snake->length = length;
    

    for (int i = 0; i < length; i++) {
        snake->nodes[i] = tail_coord;
        tail_coord = get_coord_offset(tail_coord, direction, 1);
    }
}

//Allocates snake body
void init_snake(Snake* snake) {
    snake->max_length = GRID_SIZE.x * GRID_SIZE.y;
    snake->nodes = malloc(sizeof(iVec2D) * snake->max_length);
}

//Frees snake body
void destroy_snake(Snake* snake) {
    free(snake->nodes);
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
        int idx = snake->nodes[i].x + (snake->nodes[i].y * GRID_SIZE.x);
        presence_array[idx] = true;
    }

    //start with random spot and scan through array until empty place found or everything scanned.
    int choice = GetRandomValue(0, arr_len - 1);
    int count = 1;
    while (presence_array[choice] && count < arr_len) {
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
void draw_snake(Snake snake, Texture2D sprite_sheet) {
    Direction direction;
    fVec2D screen_pos;
    fVec2D origin = {(float)SQUARE_PIXEL_WIDTH/2, (float)SQUARE_PIXEL_WIDTH/2};
    // Vector2 origin = {0.0f, 0.0f};
    float rotation = 0.0f;
    bool flip_x = false;
    bool flip_y = false;
    SnakeSpriteIdx sprite_index;
    Rectangle sprite_rect;
    Rectangle dest_rect;
    
    //Draw tail
    iVec2D tail = snake.nodes[0];
    iVec2D next = snake.nodes[1];
    iVec2D prev = {0};
    direction = get_snake_node_direction(tail, next);
    screen_pos = GridToPixelCoords(tail.x, tail.y);
    switch (direction) {
        case DIR_UP:
            sprite_index = SNAKE_TAIL_V;
            break;
        case DIR_RIGHT:
            sprite_index = SNAKE_TAIL_H;
            flip_x = true;
            break;
        case DIR_DOWN:
            sprite_index = SNAKE_TAIL_V;
            flip_y = true;
            break;
        case DIR_LEFT:
            sprite_index = SNAKE_TAIL_H;
            break;
    }
    sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH, flip_x, flip_y);
    // DrawTextureRec(sprite_sheet, sprite_rect, screen_pos, WHITE);
    dest_rect = screen_to_dest(screen_pos, sprite_rect);
    DrawTexturePro(sprite_sheet, sprite_rect, dest_rect, origin, rotation, WHITE);

    //Draw body
    flip_x = false;
    flip_y = false;
    for (int i = 1; i < snake.length - 1; i++) {
        iVec2D part = snake.nodes[i];
        prev = snake.nodes[i-1];
        next = snake.nodes[i+1];
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
        sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH, flip_x, flip_y);
        dest_rect = screen_to_dest(screen_pos, sprite_rect);
        DrawTexturePro(sprite_sheet, sprite_rect, dest_rect, origin, rotation, WHITE);
    }
    
    //Draw head
    iVec2D head = snake.nodes[snake.length - 1];
    prev = snake.nodes[snake.length - 2];
    direction = get_snake_node_direction(prev, head);
    screen_pos = GridToPixelCoords(head.x, head.y);
    flip_x = false;
    flip_y = false;
    switch (direction) {
        case DIR_UP:
            sprite_index = SNAKE_HEAD_V;
            // rotation = 0.0f;
            break;
        case DIR_RIGHT:
            sprite_index = SNAKE_HEAD_H;
            break;
        case DIR_DOWN:
            sprite_index = SNAKE_HEAD_V;
            flip_y = true;
            break;
        case DIR_LEFT:
            sprite_index = SNAKE_HEAD_H;
            flip_x = true;
            break;
    }
    sprite_rect = GetSpriteRect(sprite_index, SQUARE_PIXEL_WIDTH, flip_x, flip_y);
    dest_rect = screen_to_dest(screen_pos, sprite_rect);
    DrawTexturePro(sprite_sheet, sprite_rect, dest_rect, origin, rotation, WHITE);
}

//Grows head by one in the direction it's facing
//This is so close to the same as move_snake, wonder if we can merge...
void grow_snake(Snake* snake, Direction queued_dir) {
    if (queued_dir != DIR_NULL && queued_dir != flip_direction(snake->facing)) {
        snake->facing = queued_dir;
    }
    if (snake->length + 1 > snake->max_length) {
        return;
    }
    iVec2D head = snake->nodes[snake->length-1];
    iVec2D new_head = get_coord_offset(head, snake->facing, 1);
    snake->nodes[snake->length] = new_head;
    snake->length += 1;
}

void handle_input(GameState* state) {
    //Compactify queue
    KeyboardKey key;
    while (key = GetKeyPressed()) {
        switch(key) {
            case KEY_SPACE:
                if (state->flags[FLAG_GAMEOVER]) {
                    state->flags[FLAG_GAMERESET] = true;
                }
                break;
            case KEY_RIGHT:
                pushb_dirqueue(&(state->dir_queue), DIR_RIGHT);
                break;
            case KEY_LEFT:
                pushb_dirqueue(&(state->dir_queue), DIR_LEFT);
                break;
            case KEY_UP:
                pushb_dirqueue(&(state->dir_queue), DIR_UP);
                break;
            case KEY_DOWN:
                pushb_dirqueue(&(state->dir_queue), DIR_DOWN);
                break;
            case KEY_P:
                state->flags[FLAG_GAME_PAUSED] = !state->flags[FLAG_GAME_PAUSED];
                break;
            case KEY_S:
                state->flags[FLAG_SHOWDEBUG] = !state->flags[FLAG_SHOWDEBUG];
                break;
            case KEY_I:
                state->flags[FLAG_INVINCIBLE] = !state->flags[FLAG_INVINCIBLE];
                break;
            case KEY_LEFT_BRACKET:
                state->tick_frames += 4.0f;
                if (state->tick_frames > MAX_TICK_WAIT_FRAMES) {
                    state->tick_frames = MAX_TICK_WAIT_FRAMES;
                }
                break;
            case KEY_RIGHT_BRACKET:
                state->tick_frames -= 4.0f;
                if (state->tick_frames < MIN_TICK_WAIT_FRAMES) {
                    state->tick_frames = MIN_TICK_WAIT_FRAMES;
                }
                break;
        }
    }
}


void draw_food(iVec2D food_pos, Texture2D sprite_sheet) {
    fVec2D screen_pos = GridToPixelCoords(food_pos.x, food_pos.y);
    Rectangle sprite_rect = GetSpriteRect(FOOD_CHERRY, SQUARE_PIXEL_WIDTH, false, false);
    DrawTextureRec(sprite_sheet, sprite_rect, screen_pos, WHITE);
}

void init_state(GameState* state) {
    state->textures = malloc(sizeof(Texture2D) * SPRITE_SHEET_COUNT);
    state->textures[SPRITE_FOOD_IDX] = LoadTexture("assets/food_spritesheet.bmp");
    state->textures[SPRITE_SNAKE_IDX] = LoadTexture("assets/snake_spritesheet.bmp");
    state->textures[SPRITE_BACKGROUND_IDX] = LoadTexture("assets/backgrounds_spritesheet.bmp");
    // state->background = BACKGROUND_DIRT;
    
    state->flags = malloc(sizeof(bool) * FLAGS_COUNT);
    init_dirqueue(&(state->dir_queue), KEY_QUEUE_LENGTH);

}

void setup_game(GameState* state, Snake* snake, iVec2D* cherry_pos) {
    for (int i = 0; i < FLAGS_COUNT; i++) {
        state->flags[i] = false;
    }
    state->flags[FLAG_BACKGROUNDCHANGE] = true;
    state->background = BACKGROUND_DIRT;
    state->tick_frames = 8.0f;
    state->tick_time = GetTime();
    while (popf_dirqueue(&state->dir_queue)) {}
    set_snake(snake, (iVec2D){1,1}, DIR_DOWN, 2);
    spawn_cherry(snake, cherry_pos);
}

//Returns true if snake head is colliding with the cherry
bool cherry_collision(Snake* snake, iVec2D cherry_pos) {
    iVec2D head = snake->nodes[snake->length - 1];
    if (head.x == cherry_pos.x && head.y == cherry_pos.y) {
        return true;
    }
    return false;
}

//Returns true if snake head is colliding with some other body part
bool self_collision(Snake* snake) {
    iVec2D head = snake->nodes[snake->length - 1];
    for (int i = 0; i < snake->length - 1; i++) {
        iVec2D bod = snake->nodes[i];
        if (head.x == bod.x && head.y == bod.y) {
            return true;
        }
    }
    return false;
}



void update_game(GameState* state, Snake* snake, iVec2D* cherry_pos) {
    if (state->flags[FLAG_GAMERESET]) {
        setup_game(state, snake, cherry_pos);
    }
    if (state->flags[FLAG_GAME_PAUSED] || state->flags[FLAG_GAMEOVER]) {
        return;
    }
    if (GetTime() - state->tick_time < state->tick_frames/GAME_FPS) {
        return;
    }
    state->tick_time = GetTime();

    if (state->flags[FLAG_ATECHERRY]) {
        grow_snake(snake, popf_dirqueue(&(state->dir_queue)));
        state->flags[FLAG_ATECHERRY] = false;
    } else {
        move_snake(snake, popf_dirqueue(&(state->dir_queue)));
    }

    if (cherry_collision(snake, *cherry_pos)) {
        state->flags[FLAG_ATECHERRY] = true;
        bool spawned = spawn_cherry(snake, cherry_pos);
        if (!spawned) { //no spots for cherry to spawn, we win!
            state->flags[FLAG_GAMEOVER] = true;
            state->flags[FLAG_GAMEWIN] = true;
        }
    }
    if (self_collision(snake)) {
        if (!state->flags[FLAG_INVINCIBLE]) {
            state->flags[FLAG_GAMEOVER] = true;
            state->flags[FLAG_GAMEWIN] = false;
        }
    }
}

void draw_background(Texture2D sprite_sheet, BackgroundSpriteIdx background) {

    Rectangle background_rect = GetSpriteRect(background, SQUARE_PIXEL_WIDTH, false, false);
    for (int x = 0; x < GRID_SIZE.x; x++) { //Background
        for (int y = 0; y < GRID_SIZE.y; y++) {
            Vector2 pos = {x * SQUARE_PIXEL_WIDTH, y * SQUARE_PIXEL_WIDTH};
            DrawTextureRec(sprite_sheet, background_rect, pos, WHITE);
        }
    }
}

void draw_overlays(GameState* state, Snake snake, iVec2D cherry_pos){
    if (state->flags[FLAG_GAMEOVER]) {
        const char* msg = "GAME OVER";
        Font font = GetFontDefault();
        int fontsize = 30;
        int spacing = 0;
        Color col_overlay = {100,100,100,150};
        DrawRectangle(0, 0, SCREEN_SIZE.x, SCREEN_SIZE.y, col_overlay);
        Vector2 textsize = MeasureTextEx(font, msg, fontsize, spacing);
        Vector2 position = {SCREEN_SIZE.x/2 - textsize.x/2, SCREEN_SIZE.y/2 - textsize.y/2};
        DrawTextEx(font, msg, position, fontsize, 0.0f, WHITE);
    }

    if (state->flags[FLAG_SHOWDEBUG]) {
        int max_length = 1000;
        char stat_text[max_length];
        int stat_count = 10;
        const char* stat_strings[] = {
            "snake_length: %d\n\n",
            "cherry_pos: %d,%d\n\n",
            "tick_frames: %2.1f\n\n",
            "flag_invincible: %d\n\n",
            "flag_paused: %d\n\n",
            "flag_gameover: %d\n\n",
            "flag_gamewin: %d\n\n"
        };
        int len = 0;
        len += snprintf(stat_text, max_length, stat_strings[0], snake.length);
        len += snprintf(stat_text + len, max_length, stat_strings[1], cherry_pos.x, cherry_pos.y);
        len += snprintf(stat_text + len, max_length, stat_strings[2], state->tick_frames);
        len += snprintf(stat_text + len, max_length, stat_strings[3], state->flags[FLAG_INVINCIBLE]);
        len += snprintf(stat_text + len, max_length, stat_strings[4], state->flags[FLAG_GAME_PAUSED]);
        len += snprintf(stat_text + len, max_length, stat_strings[5], state->flags[FLAG_GAMEOVER]);
        len += snprintf(stat_text + len, max_length, stat_strings[6], state->flags[FLAG_GAMEWIN]);

        DrawText(stat_text, 10, 10, 24, RED);
    }
}

//TODO:: parse cmd line to get screen w/h and vsync/framerate
int main(char** argv, int argc) {
    InitWindow(SCREEN_SIZE.x,SCREEN_SIZE.y, "c-snake");
    SetTargetFPS(GAME_FPS);
    
    GameState state;
    Snake snake;
    iVec2D cherry_pos;
    
    init_state(&state);
    init_snake(&snake);
    setup_game(&state, &snake, &cherry_pos);

    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)SCREEN_SIZE.x / (GRID_SIZE.x * SQUARE_PIXEL_WIDTH) ; //base on screen width only right now.
    Camera2D test_cam = {0};
    test_cam.zoom = 5.0f;

    RenderTexture2D background = LoadRenderTexture(GRID_SIZE.x * SQUARE_PIXEL_WIDTH, GRID_SIZE.y * SQUARE_PIXEL_WIDTH);
    RenderTexture2D target = LoadRenderTexture(SCREEN_SIZE.x, SCREEN_SIZE.y);
    while (!WindowShouldClose()) {
        
        handle_input(&state);
        update_game(&state, &snake, &cherry_pos);

        //recreate background texture when it changes
        if (state.flags[FLAG_BACKGROUNDCHANGE]) {
            BeginTextureMode(background);
            draw_background(state.textures[SPRITE_BACKGROUND_IDX], state.background);
            EndTextureMode();
            state.flags[FLAG_BACKGROUNDCHANGE] = false;
        }

        //render to main target
        BeginTextureMode(target);
        BeginMode2D(camera);
        DrawTextureRec(background.texture, (Rectangle){0, 0, (float)background.texture.width, (float)background.texture.height}, (Vector2){0,0}, WHITE);
        draw_food(cherry_pos, state.textures[SPRITE_FOOD_IDX]);
        draw_snake(snake, state.textures[SPRITE_SNAKE_IDX]);

        EndTextureMode();

        BeginDrawing();
        DrawTextureRec(target.texture, 
                (Rectangle){0, 0, (float)target.texture.width, -1 * (float)target.texture.height}, 
                (Vector2){0, 0}, 
                WHITE);
        draw_overlays(&state, snake, cherry_pos);
        EndDrawing();
    }
    CloseWindow();
    destroy_snake(&snake);

    return 0;
}

