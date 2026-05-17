#include "gui.h"
#include "arena.h"
#include "screens.h"
#include <math.h>

const float MIN_TICK_WAIT_FRAMES = 0.0f;
const float MAX_TICK_WAIT_FRAMES = 60.0f;
const int KEY_QUEUE_LENGTH = 2;



typedef enum {
    TEXTURE_SNAKE,
    TEXTURE_FOOD,
    TEXTURE_BACKGROUNDS,
    TEXTURE_COUNT
} TextureIdx;
const char *TEXTURE_NAME_STRINGS[] = {
    "assets/snake_spritesheet.bmp",
    "assets/food_spritesheet.bmp",
    "assets/backgrounds_spritesheet.bmp"
};

typedef enum Direction {
    DIR_NULL,
    DIR_UP,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT
} Direction;

typedef struct DirectionQueue{
    Direction* buf;
    int length;
    int max_length;
} DirectionQueue;

typedef struct Snake {
    iVec2D* nodes; //points at tail
    Direction facing;
    int length;
    int max_length;
} Snake;

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

typedef enum {
    GAME_FLAG_PAUSED,
    GAME_FLAG_INVINCIBLE,
    GAME_FLAG_GAMEOVER,
    GAME_FLAG_GAMEWIN,
    GAME_FLAG_ATECHERRY,
    GAME_FLAG_BACKGROUNDCHANGE,
    GAME_FLAG_GAMERESET,
    GAME_FLAG_SHOWDEBUG,
    GAME_FLAG_COUNT
} GameFlag;


typedef struct GameState {
    DirectionQueue dir_queue;
    Snake snake;
    Texture2D* textures;
    Arena* arena;
    bool* presence_array;
    UiContext menu_gameover;
    Camera2D camera;
    BackgroundSprite bg_sprite;
    RenderTexture2D target_bg;
    RenderTexture2D target_final;
    iVec2D cherry_xy;
    iVec2D grid_size;
    double tick_time;
    double tick_frames;
    bool flags[GAME_FLAG_COUNT];
} GameState;


/*  --------------------------------------------------------------------------------------- /
                                    Utility Functions
    --------------------------------------------------------------------------------------- */

    //subtracts result of (first - second)
iVec2D sub_iVec2D(iVec2D first, iVec2D second) {
    iVec2D result;
    result.x = first.x - second.x;
    result.y = first.y - second.y;
    return result;
}


//If a coordinate is outside the bounds of the grid, this will wrap it back into bounds.
void wrap_coordinate(iVec2D* coordinate, iVec2D grid_size) {
    coordinate->x = coordinate->x % grid_size.x;
    coordinate->y = coordinate->y % grid_size.y;
    if (coordinate->x < 0) {coordinate->x += grid_size.x;}
    if (coordinate->y < 0) {coordinate->y += grid_size.y;}
}

//Returns the coordinate distance away from the start in the direction specified.
//Will wrap the coordinate based on grid sizes
iVec2D get_coord_offset(iVec2D start, Direction direction, int distance, iVec2D grid_size) {
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
    wrap_coordinate(&coord, grid_size);
    return coord;
}


/*  --------------------------------------------------------------------------------------- /
                                    Input Handling
    --------------------------------------------------------------------------------------- */

Direction flip_direction(Direction d) {
    switch (d) {
        case DIR_UP: return DIR_DOWN;
        case DIR_RIGHT: return DIR_LEFT;
        case DIR_DOWN: return DIR_UP;
        case DIR_LEFT: return DIR_RIGHT;
    }
}

void init_dirqueue(DirectionQueue* queue, int maxlength, Arena* arena) {
    queue->length = 0;
    queue->max_length = maxlength;
    queue->buf = arena_alloc(arena, sizeof(Direction) * maxlength);
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

void handle_input(GameState* state) {
    KeyboardKey key;
    while (key = GetKeyPressed()) {
        switch(key) {
            case KEY_SPACE:
                if (state->flags[GAME_FLAG_GAMEOVER]) {
                    state->flags[GAME_FLAG_GAMERESET] = true;
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
                state->flags[GAME_FLAG_PAUSED] = !state->flags[GAME_FLAG_PAUSED];
                break;
            case KEY_S:
                state->flags[GAME_FLAG_SHOWDEBUG] = !state->flags[GAME_FLAG_SHOWDEBUG];
                break;
            case KEY_I:
                state->flags[GAME_FLAG_INVINCIBLE] = !state->flags[GAME_FLAG_INVINCIBLE];
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


/*  --------------------------------------------------------------------------------------- /
                                    Cherry Functions
    --------------------------------------------------------------------------------------- */

//Pick random unoccupied location to spawn the cherry.
//If there's an empty spot, cherry_location is populated and function returns true
//If there are no empty locations, returns false
bool spawn_cherry(GameState* gs) {
    //Set up bool array with snake locations true, all else false
    const int arr_len = gs->grid_size.x * gs->grid_size.y;
    for (int i = 0; i < arr_len; i++) { gs->presence_array[i] = false; }
    
    for (int i = 0; i < gs->snake.length; i++) {
        int idx = gs->snake.nodes[i].x + (gs->snake.nodes[i].y * gs->grid_size.x);
        gs->presence_array[idx] = true;
    }

    //start with random spot and scan through array until empty place found or everything scanned.
    int choice = GetRandomValue(0, arr_len - 1);
    int count = 1;
    while (gs->presence_array[choice] && count < arr_len) {
        choice++;
        count++;
        if (choice >= arr_len) {choice = 0;}
    }

    //If it's empty, populate coordinates. Otherwise return false.
    if (!gs->presence_array[choice]) {
        gs->cherry_xy.x = choice % gs->grid_size.x;
        gs->cherry_xy.y = choice / gs->grid_size.x;
        return true;
    }
    return false;
}

//Returns true if snake head is colliding with the cherry
bool cherry_collision(Snake* snake, iVec2D cherry_pos) {
    iVec2D head = snake->nodes[snake->length - 1];
    if (head.x == cherry_pos.x && head.y == cherry_pos.y) {
        return true;
    }
    return false;
}

/*  --------------------------------------------------------------------------------------- /
                                    Snake Functions
    --------------------------------------------------------------------------------------- */

//Updates snake to be a straight line in direction provided starting at tail coordinate.
void set_snake(Snake* snake, iVec2D tail_coord, Direction direction, int length, iVec2D grid_size) {
    snake->facing = direction;
    snake->length = length;
    

    for (int i = 0; i < length; i++) {
        snake->nodes[i] = tail_coord;
        tail_coord = get_coord_offset(tail_coord, direction, 1, grid_size);
    }
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

//Allocates snake body
void init_snake(Snake* snake, iVec2D grid_size, Arena* arena) {
    snake->max_length = grid_size.x * grid_size.y;
    
    snake->nodes = arena_alloc(arena, sizeof(iVec2D) * snake->max_length);
}

//If it's valid, updates snake's facing direction and moves by one space
void move_snake(Snake* snake, Direction queued_dir, iVec2D grid_size) {
    if (queued_dir != DIR_NULL && queued_dir != flip_direction(snake->facing)) {
        snake->facing = queued_dir;
    }

    for (int i = 0; i < snake->length - 1; i++) {
        snake->nodes[i] = snake->nodes[i+1];
    }
    iVec2D* head = &snake->nodes[snake->length - 1];
    iVec2D new_head = get_coord_offset(*head, snake->facing, 1, grid_size);
    *head = new_head;
}

//Grows head by one in the direction it's facing
//This is so close to the same as move_snake, wonder if we can merge...
void grow_snake(Snake* snake, Direction queued_dir, iVec2D grid_size) {
    if (queued_dir != DIR_NULL && queued_dir != flip_direction(snake->facing)) {
        snake->facing = queued_dir;
    }
    if (snake->length + 1 > snake->max_length) {
        return;
    }
    iVec2D head = snake->nodes[snake->length-1];
    iVec2D new_head = get_coord_offset(head, snake->facing, 1, grid_size);
    snake->nodes[snake->length] = new_head;
    snake->length += 1;
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


/*  --------------------------------------------------------------------------------------- /
                                    Menu Functions
    --------------------------------------------------------------------------------------- */

void restart_clicked(UiContext* ctx, UiElement* elem) {
    SceneState* ss = (SceneState*)elem->element_data;
    ss->next_scene = SCENE_GAME;
    ss->flags[SCENE_FLAG_SCENECHANGE] = true;
}

void mainmenu_clicked(UiContext* ctx, UiElement* elem) {
    SceneState* ss = (SceneState*)elem->element_data;
    ss->next_scene = SCENE_MENU;
    ss->flags[SCENE_FLAG_SCENECHANGE] = true;
}

void setup_gameover(GameState* gs, SceneState* ss) {
    UiContext* ctx = &gs->menu_gameover;
    setup_uicontext(ctx);
    // ctx->default_theme->text_font.baseSize = 10;
    for (int i = 0; i < ELEM_STATE_COUNT; i++) {
        set_theme_float_attr(ctx->default_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 30.0f);
    }
    
    int elem_w = 200;
    int elem_h = 80;

    UiElement* restart_button = element_create(ctx);
    int elem_x = ss->persist_data->screen_size.x / 2 - elem_w / 2;
    int elem_y = ss->persist_data->screen_size.y / 2 - elem_h;
    restart_button->type = ELEM_BUTTON;
    restart_button->text = "RESTART";
    restart_button->draw_rect = true;
    restart_button->rect = (Rectangle){elem_x, elem_y, 200, 80};
    restart_button->element_data = ss;
    restart_button->click_action = restart_clicked;

    UiElement* mainmenu_button = element_create(ctx);
    elem_y = ss->persist_data->screen_size.y / 2 + elem_h;
    mainmenu_button->type = ELEM_BUTTON;
    mainmenu_button->text = "MAIN MENU";
    mainmenu_button->draw_rect = true;
    mainmenu_button->rect = (Rectangle){elem_x, elem_y, 200, 80};
    mainmenu_button->element_data = ss;
    mainmenu_button->click_action = mainmenu_clicked;
}


/*  --------------------------------------------------------------------------------------- /
                                    Drawing Functions
    --------------------------------------------------------------------------------------- */

//Translates screen position and texture rectangle to a destination rectangle.
//Note that it also offsets the screen position to center the texture (makes rotation easier later)
Rectangle screen_to_dest(Vector2 screen_position, Rectangle texture_rectangle) {
    float half_width = (float)SQUARE_PIXEL_WIDTH/2;
    Rectangle dest = {screen_position.x + half_width, screen_position.y + half_width, fabsf(texture_rectangle.width), fabsf(texture_rectangle.height)};
    return dest;
}

Vector2 GridToPixelCoords(int grid_x, int grid_y) {
    Vector2 coord = {grid_x * SQUARE_PIXEL_WIDTH, grid_y * SQUARE_PIXEL_WIDTH};
    return coord;
}

//Draws the snake to the screen
void draw_snake(Snake snake, Texture2D sprite_sheet) {
    Direction direction;
    Vector2 screen_pos;
    Vector2 origin = {(float)SQUARE_PIXEL_WIDTH/2, (float)SQUARE_PIXEL_WIDTH/2};
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

void draw_food(iVec2D food_pos, Texture2D sprite_sheet) {
    Vector2 screen_pos = GridToPixelCoords(food_pos.x, food_pos.y);
    Rectangle sprite_rect = GetSpriteRect(FOOD_CHERRY, SQUARE_PIXEL_WIDTH, false, false);
    DrawTextureRec(sprite_sheet, sprite_rect, screen_pos, WHITE);
}


/*  --------------------------------------------------------------------------------------- /
                                    Scene Callbacks
    --------------------------------------------------------------------------------------- */

void setup_gamescreen(SceneState* state) {
    SetWindowSize(state->persist_data->screen_size.x, state->persist_data->screen_size.y);
    
    GameState* gs = malloc(sizeof(GameState));
    *gs = (GameState){};
    arena_init(&gs->arena);
    
    gs->grid_size = state->persist_data->grid_size;
    gs->bg_sprite = state->persist_data->selected_background;
    gs->textures = arena_alloc(gs->arena, sizeof(Texture2D) * TEXTURE_COUNT);
    gs->presence_array = arena_alloc(gs->arena, gs->grid_size.x * gs->grid_size.y);
    for (int i = TEXTURE_SNAKE; i < TEXTURE_COUNT; i++) {
        gs->textures[i] = LoadTexture(TEXTURE_NAME_STRINGS[i]);
    }
    gs->tick_frames = 8.0f;
    gs->tick_time = GetTime();

    
    //init snake
    init_dirqueue(&gs->dir_queue, 2, gs->arena);
    init_snake(&gs->snake, state->persist_data->grid_size, gs->arena);
    set_snake(&gs->snake, (iVec2D){1,1}, DIR_DOWN, 2, gs->grid_size);
    spawn_cherry(gs);
    
    //render targets
    gs->target_bg = LoadRenderTexture(gs->grid_size.x * SQUARE_PIXEL_WIDTH, gs->grid_size.y * SQUARE_PIXEL_WIDTH);
    gs->target_final = LoadRenderTexture(state->persist_data->screen_size.x, state->persist_data->screen_size.y);
    gs->flags[GAME_FLAG_BACKGROUNDCHANGE] = true;

    //setup camera
    gs->camera = (Camera2D){0};
    float screen_size_x = (float)state->persist_data->screen_size.x;
    gs->camera.zoom = screen_size_x / (gs->grid_size.x * SQUARE_PIXEL_WIDTH);

    //setup menu screen for game over
    setup_gameover(gs, state);
    
    state->screen_memory = gs;
}

void unload_gamescreen(SceneState* state) {
    GameState* gs = (GameState*)state->screen_memory;
    destroy_uicontext(&gs->menu_gameover);
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        UnloadTexture(gs->textures[i]);
    }
    arena_destroy(&gs->arena);
    free(gs);
}

void update_gamescreen(SceneState* scene) {
    GameState* gs = (GameState*)scene->screen_memory;
    handle_input(gs);

    if (gs->flags[GAME_FLAG_GAMERESET]) {
        scene->next_scene = SCENE_GAME;
        scene->flags[SCENE_FLAG_SCENECHANGE] = true;
        return;
    }

    if (gs->flags[GAME_FLAG_PAUSED]) {
        return;
    }

    if (gs->flags[GAME_FLAG_GAMEOVER]) {
        update_uicontext(&gs->menu_gameover);
        return;
    }
    
    if (GetTime() - gs->tick_time < gs->tick_frames/scene->persist_data->game_fps) {
        return;
    }
    gs->tick_time = GetTime();

    if (gs->flags[GAME_FLAG_ATECHERRY]) {
        grow_snake(&gs->snake, popf_dirqueue(&(gs->dir_queue)), gs->grid_size);
        gs->flags[GAME_FLAG_ATECHERRY] = false;
    } else {
        move_snake(&gs->snake, popf_dirqueue(&(gs->dir_queue)), gs->grid_size);
    }

    if (cherry_collision(&gs->snake, gs->cherry_xy)) {
        gs->flags[GAME_FLAG_ATECHERRY] = true;
        bool spawned = spawn_cherry(gs);
        if (!spawned) { //no spots for cherry to spawn, we win!
            gs->flags[GAME_FLAG_GAMEOVER] = true;
            gs->flags[GAME_FLAG_GAMEWIN] = true;
        }
    }
    
    if (self_collision(&gs->snake)) {
        if (!gs->flags[GAME_FLAG_INVINCIBLE]) {
            gs->flags[GAME_FLAG_GAMEOVER] = true;
            gs->flags[GAME_FLAG_GAMEWIN] = false;
        }
    }
}

void draw_gamescreen(SceneState* scene){
    //camera setup
    GameState* gs = (GameState*)scene->screen_memory;

    if (gs->flags[GAME_FLAG_BACKGROUNDCHANGE]) {
        BeginTextureMode(gs->target_bg);
        Rectangle background_rect = GetSpriteRect(gs->bg_sprite, SQUARE_PIXEL_WIDTH, false, false);
        for (int x = 0; x < gs->grid_size.x; x++) { //Background
            for (int y = 0; y < gs->grid_size.y; y++) {
                Vector2 pos = {x * SQUARE_PIXEL_WIDTH, y * SQUARE_PIXEL_WIDTH};
                DrawTextureRec(gs->textures[TEXTURE_BACKGROUNDS], background_rect, pos, WHITE);
            }
        }
        EndTextureMode();
        gs->flags[GAME_FLAG_BACKGROUNDCHANGE] = false;
    }

    //render to main target
    BeginTextureMode(gs->target_final);
    BeginMode2D(gs->camera);
    DrawTextureRec(gs->target_bg.texture, 
        (Rectangle){.width = (float)gs->target_bg.texture.width, .height = (float)gs->target_bg.texture.height}, 
        (Vector2){0,0}, WHITE);
    draw_food(gs->cherry_xy, gs->textures[TEXTURE_FOOD]);
    draw_snake(gs->snake, gs->textures[TEXTURE_SNAKE]);
    EndTextureMode();

    //Render to screen
    BeginDrawing();
    DrawTextureRec(gs->target_final.texture, 
            (Rectangle){0, 0, (float)gs->target_final.texture.width, -1 * (float)gs->target_final.texture.height}, 
            (Vector2){0, 0}, 
            WHITE);

    if (gs->flags[GAME_FLAG_GAMEOVER]) {
        DrawRectangle(0, 0, 
            scene->persist_data->screen_size.x, scene->persist_data->screen_size.y,
            (Color){130, 130, 130, 100});
        draw_uicontext(&gs->menu_gameover);
    }
    EndDrawing();
}