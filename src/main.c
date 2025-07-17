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

typedef enum {
    SCENE_GAME,
    SCENE_MENU,
    SCENE_SETTINGS //unused
} Scene;

typedef enum{
    TARGET_BACKGROUND,
    TARGET_OUTPUT,
    TARGET_COUNT
} RenderTarget;

typedef struct DirectionQueue{
    Direction* buf;
    int length;
    int max_length;
} DirectionQueue;

typedef struct GameState{
    double tick_frames;
    double tick_time; //tracks time since last tick
    iVec2D grid_size;
    DirectionQueue dir_queue;
    BackgroundSpriteIdx background;
    Scene current_scene;
    Camera2D camera;
    Texture2D textures[SPRITE_SHEET_COUNT];
    RenderTexture2D render_targets[TARGET_COUNT];
    bool flags[FLAGS_COUNT];
} GameState;

// typedef struct RectangleButton {
//     int border_thickness;
//     Rectangle rectangle;
//     Color inner_color;
//     Color border_color;
// } RectangleButton;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER
} TextAlignment;

typedef struct Snake {
    Direction facing;
    iVec2D* nodes; //points at tail
    int length;
    int max_length;
} Snake;

typedef struct UiElement{
    Rectangle rect;
    bool draw_rect; //may be false for text only
    
    int border_thickness;
    Color inner_color;
    Color border_color;

    bool draw_glow;
    int glow_thickness;
    Color glow_color;
    
    char* text;
    Color text_color;
    Font text_font;
    float text_size;
    float text_spacing;
    TextAlignment text_align;
} UiElement;


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
    // state->textures = malloc(sizeof(Texture2D) * SPRITE_SHEET_COUNT);
    state->textures[SPRITE_FOOD_IDX] = LoadTexture("assets/food_spritesheet.bmp");
    state->textures[SPRITE_SNAKE_IDX] = LoadTexture("assets/snake_spritesheet.bmp");
    state->textures[SPRITE_BACKGROUND_IDX] = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    init_dirqueue(&(state->dir_queue), KEY_QUEUE_LENGTH);
    state->current_scene = SCENE_MENU;
    state->grid_size = (iVec2D){8,8};
    
    // state->render_targets[TARGET_BACKGROUND] =LoadRenderTexture(GRID_SIZE.x * SQUARE_PIXEL_WIDTH, GRID_SIZE.y * SQUARE_PIXEL_WIDTH);
    state->render_targets[TARGET_OUTPUT] = LoadRenderTexture(SCREEN_SIZE.x, SCREEN_SIZE.y);
    // RenderTexture2D background = LoadRenderTexture(GRID_SIZE.x * SQUARE_PIXEL_WIDTH, GRID_SIZE.y * SQUARE_PIXEL_WIDTH);
    // RenderTexture2D target = LoadRenderTexture(SCREEN_SIZE.x, SCREEN_SIZE.y);

    // state->flags = malloc(sizeof(bool) * FLAGS_COUNT);

}

void destroy_state(GameState* state){
    free(state->dir_queue.buf);
    free(state);
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

    //setup camera
    //setup render target for background
    //setup grid size...?
    //set current scene
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

//set default values for UI element (zeroing where appropriate)
//DOES NOT CONSTRUCT ELEMENT
//Relies on GetFontDefault from raylib
void init_ui_element(UiElement* element) {
    element->draw_rect = true;
    element->rect = (Rectangle){0};

    element->border_thickness = 0;
    element->border_color = BLACK;
    element->inner_color = WHITE;

    element->draw_glow = false;
    element->glow_thickness = 0;
    element->glow_color = (Color){0};
    
    element->text = NULL;
    element->text_color = BLACK;
    element->text_font = GetFontDefault();
    element->text_size = 12.0f;
    element->text_spacing = 1.0f;
    element->text_align = ALIGN_CENTER;
}

//Draw UI element to current render target
void draw_uielement(UiElement* element) {
    if (element->draw_glow) {
        DrawRectangleGradientV(element->rect.x,
                    element->rect.y - element->glow_thickness,
                    element->rect.width,
                    element->glow_thickness,
                    (Color){element->glow_color.r,element->glow_color.g,element->glow_color.b,0},
                    element->glow_color);
        DrawRectangleGradientV(element->rect.x,
                    element->rect.y + element->rect.height,
                    element->rect.width,
                    element->glow_thickness,
                    element->glow_color,
                    (Color){element->glow_color.r,element->glow_color.g,element->glow_color.b,0});
        DrawRectangleGradientH(element->rect.x - element->glow_thickness,
                    element->rect.y,
                    element->glow_thickness,
                    element->rect.height,
                    (Color){element->glow_color.r,element->glow_color.g,element->glow_color.b,0},
                    element->glow_color);
        DrawRectangleGradientH(element->rect.x + element->rect.width,
                    element->rect.y,
                    element->glow_thickness,
                    element->rect.height,
                    element->glow_color,
                    (Color){element->glow_color.r,element->glow_color.g,element->glow_color.b,0});
    }

    
    if (element->draw_rect) {
        if (element->border_thickness) {
            Rectangle border_rect = (Rectangle){
                element->rect.x - element->border_thickness,
                element->rect.y - element->border_thickness,
                element->rect.width + element->border_thickness * 2,
                element->rect.height + element->border_thickness * 2
                };
            DrawRectangleRec(border_rect, element->border_color);
        }

        DrawRectangleRec(element->rect, element->inner_color);
    }
    
    if (element->text) {
        Vector2 t_sz = MeasureTextEx(element->text_font, element->text, element->text_size, element->text_spacing);
        Vector2 pos;
        switch(element->text_align) {
            case ALIGN_LEFT:
                pos.x = element->rect.x;
                pos.y = element->rect.y + element->rect.height/2 - t_sz.y / 2;
                break;
            case ALIGN_CENTER:
                pos.x = element->rect.x + element->rect.width/2 - t_sz.x/2;
                pos.y = element->rect.y + element->rect.height/2 - t_sz.y / 2;
                break;
        }

        DrawTextEx(element->text_font, 
            element->text, 
            pos, 
            element->text_size, 
            element->text_spacing, 
            element->text_color);
    }
}

//Just dumping raylib sound functions here for now
void play_sound() {
    // RLAPI void InitAudioDevice(void);                                     // Initialize audio device and context
    // RLAPI void CloseAudioDevice(void);                                    // Close the audio device and context
    // RLAPI bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
    // RLAPI void SetMasterVolume(float volume);                             // Set master volume (listener)
    // RLAPI float GetMasterVolume(void);                                    // Get master volume (listener)
    // RLAPI Wave LoadWave(const char *fileName);                            // Load wave data from file
    // RLAPI Sound LoadSound(const char *fileName);                          // Load sound from file
    // RLAPI void PlaySound(Sound sound);                                    // Play a sound
    // RLAPI void StopSound(Sound sound);                                    // Stop playing a sound
    // RLAPI void PauseSound(Sound sound);                                   // Pause a sound
    // RLAPI void ResumeSound(Sound sound);                                  // Resume a paused sound
    // RLAPI bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
    // RLAPI void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)

    // RLAPI Music LoadMusicStream(const char *fileName);                    // Load music stream from file
    // RLAPI Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize); // Load music stream from data
    // RLAPI bool IsMusicReady(Music music);                                 // Checks if a music stream is ready
    // RLAPI void UnloadMusicStream(Music music);                            // Unload music stream
    // RLAPI void PlayMusicStream(Music music);                              // Start music playing
    // RLAPI void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
    // RLAPI void StopMusicStream(Music music);                              // Stop music playing
    // RLAPI void PauseMusicStream(Music music);                             // Pause music playing
    // RLAPI void ResumeMusicStream(Music music);                            // Resume playing paused music
}

void run_menu(GameState* state, Snake* snake, iVec2D cherry_pos){
    //Input handling
    // while(GetKeyPressed()) {} //clear key buffer
    // Vector2 mouse_pos = GetMousePosition();

    //Render menu
    //TODO:: Move some of this stuff to a menu setup location
    RenderTexture2D target = state->render_targets[TARGET_OUTPUT];
    BeginTextureMode(target);
    UiElement f;
    init_ui_element(&f);
    f.rect= (Rectangle){50, 50, 150, 40};
    f.glow_thickness = 10;
    f.draw_glow = true;
    f.glow_color = (Color){10,240,0,255};
    f.border_thickness = 2;
    f.text = "Hello World";


    //Draw four rectangles for glow effect

    //make a button do a thing?
    //If mouse is hovering over element, give it a glow or something....
    //input_handling needs to store

    ClearBackground(SKYBLUE);
    draw_uielement(&f);
    EndTextureMode();

    BeginDrawing();
    DrawTextureRec(target.texture, 
                (Rectangle){0, 0, (float)target.texture.width, -1 * (float)target.texture.height}, 
                (Vector2){0, 0}, 
                WHITE);
    EndDrawing();
}

void run_game(GameState* state, Snake* snake, iVec2D cherry_pos) {
    //camera setup
    Camera2D camera = {0};
    camera.zoom = (float)SCREEN_SIZE.x / (GRID_SIZE.x * SQUARE_PIXEL_WIDTH) ; //base on screen width only right now.
    Camera2D test_cam = {0};
    test_cam.zoom = 5.0f;


        // handle_input(&state);
        // update_game(&state, &snake, &cherry_pos);

        // //recreate background texture when it changes
        // if (state.flags[FLAG_BACKGROUNDCHANGE]) {
        //     BeginTextureMode(background);
        //     draw_background(state.textures[SPRITE_BACKGROUND_IDX], state.background);
        //     EndTextureMode();
        //     state.flags[FLAG_BACKGROUNDCHANGE] = false;
        // }

        // //render to main target
        // BeginTextureMode(target);
        // BeginMode2D(camera);
        // DrawTextureRec(background.texture, (Rectangle){0, 0, (float)background.texture.width, (float)background.texture.height}, (Vector2){0,0}, WHITE);
        // draw_food(cherry_pos, state.textures[SPRITE_FOOD_IDX]);
        // draw_snake(snake, state.textures[SPRITE_SNAKE_IDX]);

        // EndTextureMode();

        // BeginDrawing();
        // DrawTextureRec(target.texture, 
        //         (Rectangle){0, 0, (float)target.texture.width, -1 * (float)target.texture.height}, 
        //         (Vector2){0, 0}, 
        //         WHITE);
        // draw_overlays(&state, snake, cherry_pos);
        // EndDrawing();

}

//TODO:: parse cmd line to get screen w/h and vsync/framerate
int main(char** argv, int argc) {
    InitWindow(SCREEN_SIZE.x,SCREEN_SIZE.y, "c-snake");
    SetTargetFPS(GAME_FPS);
    
    GameState* state = malloc(sizeof(GameState));
    Snake snake;
    iVec2D cherry_pos;
    
    init_state(state);
    init_snake(&snake);
    // setup_game(state, &snake, &cherry_pos);
    // Scene curScene = SCENE_MENU;


    while (!WindowShouldClose()) {
        switch(state->current_scene) {
            case SCENE_MENU:
                run_menu(state, &snake, cherry_pos);
                break;
                //handle clicks/input
                //render menu
                //Once selections are made, need to setup the game.
            case SCENE_GAME:
                run_game(state, &snake, cherry_pos);
                break;
        }
        
    }
    CloseWindow();
    destroy_snake(&snake);
    destroy_state(state);

    return 0;
}

