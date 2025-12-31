#include "screens.h"
#include "gui.h"
#include <stdlib.h> //malloc, NULL
#include <string.h>

//TODO:: Move elsewhere, commons maybe
#define SQUARE_PIXEL_WIDTH 16

typedef enum{
    TARGET_BACKGROUND,
    TARGET_OUTPUT,
    TARGET_COUNT
} RenderTarget;

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

typedef enum {
    MAP_SMALLSIZE,
    MAP_MEDIUMSIZE,
    MAP_LARGESIZE,
    MAP_SIZECOUNT
} MapSize;
static const char* MAP_SIZE_STRINGS[] = {"SMALL","MEDIUM","LARGE"};

typedef enum {
    RES_800x800,
    RES_1200x1200,
    RES_1600x1600,
    RES_2080x2080,
    RES_COUNT
} ScreenRes;
static const char* SCREEN_RES_STRINGS[] = {"800x800","1200x1200","1600x1600","2080x2080"};

typedef struct {
    UiContext gui;
    UiComboBox* map_sizes;
    UiComboBox* map_textures;
    UiComboBox* map_resolutions;
    UiElement* error_text;
    Texture2D t2d_background; //TODO:: remove?
    bool menu_finished;
} MenuGui; //TODO:: change to menu screen object


Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    rect.width = flip_x ? -1 * rect.width : rect.width;
    rect.height = flip_y ? -1 * rect.height : rect.height;
    return rect;
}

void start_click(UiContext* ctx, UiElement* elem) {
    MenuGui* menu = (MenuGui*)elem->element_data;
    elem->state = ELEM_DEFAULT;

    if (!menu->map_resolutions->selected) {
        menu->error_text->text = "Must select a resolution.";
        return;
    } else if (!menu->map_sizes->selected) {
        menu->error_text->text = "Must select a grid size.";
        return;
    } else if (!menu->map_textures->selected) {
        menu->error_text->text = "Must select a texture.";
        return;
    }

    menu->error_text->text = NULL;
    menu->menu_finished = true;
}

void setup_menu(MenuGui* menu, GameState* state) {
    UiContext* uictx = &menu->gui;
    setup_uicontext(uictx);
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    menu->menu_finished = false;
    
    //Change themes
    UiTheme* default_theme = uitheme_getdefault(uictx);
    UiTheme* start_theme = uitheme_createcopy(uictx, default_theme);
    for (int i = 0; i < ELEM_STATE_COUNT; i++) {
        set_theme_int_attr(default_theme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_LEFT);
        set_theme_int_attr(default_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_BORDER_THICKNESS, 4);
        set_theme_float_attr(default_theme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SIZE, 40.0);
        set_theme_float_attr(default_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 30.0);
        set_theme_color_attr(default_theme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, BLUE);

        set_theme_float_attr(start_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 30.0);
        set_theme_int_attr(start_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);
    }
    set_theme_color_attr(start_theme, ELEM_DEFAULT, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, ORANGE);
    set_theme_color_attr(start_theme, ELEM_SELECTED, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, ORANGE);
    set_theme_color_attr(default_theme, ELEM_SELECTED, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, WHITE);

    const int elem_ygap = 40;
    const int elem_h = 50;
    const int text_lx = 20;
    const int box_lx = 80;
    const int box_w = 230;
    const int box_xsp = 40;
    
    //Map resolutions
    int elem_x = text_lx;
    int elem_y = 40;
    UiElement* resolution_text = element_create(uictx);
    resolution_text->type = ELEM_TEXTBOX;
    resolution_text->text = "Game Resolution:";
    resolution_text->rect = (Rectangle){.x = elem_x, .y = elem_y};

    elem_x = box_lx;
    elem_y += elem_ygap;
    menu->map_resolutions = combobox_create(uictx, RES_COUNT);
    for (int i = 0; i < RES_COUNT; i++) {
        menu->map_resolutions->elements[i].type = ELEM_BUTTON;
        menu->map_resolutions->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = elem_h};
        menu->map_resolutions->elements[i].draw_rect = true;
        menu->map_resolutions->elements[i].text = SCREEN_RES_STRINGS[i];

        elem_x += box_w + box_xsp;
    }

    //Grid sizes
    elem_x = text_lx;
    elem_y += elem_h + elem_ygap;
    UiElement* map_sizetext = element_create(uictx);
    map_sizetext->type = ELEM_TEXTBOX;
    map_sizetext->text = "Grid Size:";
    map_sizetext->rect = (Rectangle){.x = elem_x, .y = elem_y};

    elem_x = box_lx;
    elem_y += elem_ygap;
    menu->map_sizes = combobox_create(uictx, MAP_SIZECOUNT);
    for (int i = 0; i < MAP_SIZECOUNT; i++) {
        menu->map_sizes->elements[i].type = ELEM_BUTTON;
        menu->map_sizes->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = elem_h};
        menu->map_sizes->elements[i].draw_rect = true;
        menu->map_sizes->elements[i].text = MAP_SIZE_STRINGS[i];

        elem_x += box_w + box_xsp;
    }

    //Backgrounds
    elem_x = text_lx;
    elem_y += elem_h + elem_ygap;
    UiElement* texture_text = element_create(uictx);
    texture_text->type = ELEM_TEXTBOX;
    texture_text->text = "Backgrounds:";
    texture_text->rect = (Rectangle){.x = elem_x, .y = elem_y};

    elem_x = box_lx;
    elem_y += elem_ygap;
    menu->map_textures = combobox_create(uictx, BACKGROUND_COUNT);
    for (int i = 0; i < BACKGROUND_COUNT; i++) {
        menu->map_textures->elements[i].type = ELEM_BUTTON;
        menu->map_textures->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = box_w};
        menu->map_textures->elements[i].draw_rect = true;
        menu->map_textures->elements[i].inner_texture = menu->t2d_background;
        menu->map_textures->elements[i].texture_rect = GetSpriteRect(i, SQUARE_PIXEL_WIDTH, false, false);
        menu->map_textures->elements[i].text = BACKGROUNDS_STRINGS[i];
        
        elem_x += box_w + box_xsp;
    }
    
    //Start button
    const int start_w = 200;
    elem_y += box_w + elem_ygap;
    elem_x = state->screen_size.x/2 - start_w/2;
    UiElement* start_button = element_create(uictx);
    start_button->theme = start_theme;
    start_button->type = ELEM_BUTTON;
    start_button->draw_rect = true;
    start_button->click_action = start_click;
    start_button->rect = (Rectangle){.x = elem_x, .y = elem_y, .width = start_w, .height = elem_h};
    start_button->text = "START";
    start_button->element_data = menu;

    //Error string
    elem_y += elem_h + elem_ygap;
    elem_x = text_lx;
    UiElement* error_text = element_create(uictx);
    error_text->type = ELEM_TEXTBOX;
    error_text->rect = (Rectangle){.x = elem_x, .y = elem_y};
    error_text->visible = true;
    error_text->theme = uitheme_createcopy(uictx, default_theme);
    set_theme_color_attr(error_text->theme, ELEM_DEFAULT, ELEM_TEXTBOX, ELEM_COLOR_ATTR_TEXT_COLOR, RED);
    menu->error_text = error_text;
}

void setup_menuscreen(GameState* state) {
    state->screen_size = (iVec2D){1200,1000};
    SetWindowSize(state->screen_size.x, state->screen_size.y);
    
    MenuGui* menu = malloc(sizeof(MenuGui));
    setup_menu(menu, state);
    state->screen_memory = menu;
}


void update_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    update_uicontext(&menu->gui);
    if (menu->menu_finished) {
        state->flags[FLAG_SCENECHANGE] = true;
        state->next_scene = SCENE_GAME;
    }
}

void draw_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    BeginDrawing();
    ClearBackground(SKYBLUE);
    
    draw_uicontext(&menu->gui);

    EndDrawing();
}

void unload_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    UnloadTexture(menu->t2d_background);
    
    ScreenRes resolution = menu->map_resolutions->selected - menu->map_resolutions->elements;
    MapSize map_size = menu->map_sizes->selected - menu->map_sizes->elements;
    BackgroundSprite background = menu->map_textures->selected - menu->map_textures->elements;

    switch (resolution) {
        case RES_800x800:
            state->screen_size = (iVec2D){800,800};
            break;
        case RES_1200x1200:
            state->screen_size = (iVec2D){1200,1200};
            break;
        case RES_1600x1600:
            state->screen_size = (iVec2D){1600,1600};
            break;
        case RES_2080x2080:
            state->screen_size = (iVec2D){2080,2080};
            break;
    }

    switch (map_size) {
        case MAP_SMALLSIZE:
            state->grid_size = (iVec2D){.x = 20, .y = 20};
            break;
        case MAP_MEDIUMSIZE:
            state->grid_size = (iVec2D){.x = 50, .y = 50};
            break;
        case MAP_LARGESIZE:
            state->grid_size = (iVec2D){.x = 100, .y = 100};
            break;
    }
    destroy_uicontext(&menu->gui);
    free(menu);
}