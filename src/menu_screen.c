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

    UiElement* mouse_overelem;
    UiElement* mouse_downelem;
    UiElement* clicked_elem;

    UiElement* size_text;
    UiComboBox map_sizes;

    UiElement* background_text;
    UiComboBox map_backgrounds;

    UiElement* resolution_text;
    UiComboBox resolutions;
    
    UiElement* start_button;
    
    Texture2D t2d_background; //TODO:: remove?
} MenuGui; //TODO:: change to menu screen object


Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    rect.width = flip_x ? -1 * rect.width : rect.width;
    rect.height = flip_y ? -1 * rect.height : rect.height;
    return rect;
}

void start_click(UiContext* ctx, UiElement* elem) {
    set_theme_color_attr(elem->theme, ELEM_DEFAULT, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, ORANGE);
    elem->state = ELEM_DEFAULT;
    //Trigger menu exit and transition to game start.
}

void setup_menu(MenuGui* menu, GameState* state) {
    UiContext* uictx = &menu->gui;
    setup_uicontext(uictx);
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    //Change themes
    UiTheme* default_theme = uitheme_getdefault(uictx);
    UiTheme* start_theme = uitheme_createcopy(uictx, default_theme);
    for (int i = 0; i < ELEM_STATE_COUNT; i++) {
        set_theme_int_attr(default_theme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_LEFT);
        set_theme_float_attr(default_theme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SIZE, 40.0);
        set_theme_float_attr(default_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 30.0);
        set_theme_color_attr(default_theme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, BLUE);

        set_theme_float_attr(start_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 30.0);
        set_theme_int_attr(start_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);

    }
    set_theme_color_attr(default_theme, ELEM_SELECTED, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, WHITE);

    const int elem_ygap = 40;
    const int elem_h = 50;
    const int text_lx = 20;
    const int box_lx = 80;
    const int box_w = 230;
    const int box_xsp = 40;
    
    int elem_x = text_lx;
    int elem_y = 40;
    
    //Map resolutions
    UiElement* resolution_text = element_create(uictx);
    resolution_text->type = ELEM_TEXTBOX;
    resolution_text->text = "Game Resolution:";
    resolution_text->rect = (Rectangle){.x = elem_x, .y = elem_y};

    elem_x = box_lx;
    elem_y += elem_ygap;

    UiComboBox* resolutions = combobox_create(uictx, RES_COUNT);
    for (int i = 0; i < RES_COUNT; i++) {
        resolutions->elements[i].type = ELEM_BUTTON;
        resolutions->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = elem_h};
        resolutions->elements[i].draw_rect = true;
        resolutions->elements[i].text = SCREEN_RES_STRINGS[i];

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

    UiComboBox* map_sizes = combobox_create(uictx, MAP_SIZECOUNT);
    for (int i = 0; i < MAP_SIZECOUNT; i++) {
        map_sizes->elements[i].type = ELEM_BUTTON;
        map_sizes->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = elem_h};
        map_sizes->elements[i].draw_rect = true;
        map_sizes->elements[i].text = MAP_SIZE_STRINGS[i];

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

    UiComboBox* map_textures = combobox_create(uictx, BACKGROUND_COUNT);
    for (int i = 0; i < BACKGROUND_COUNT; i++) {
        map_textures->elements[i].type = ELEM_BUTTON;
        map_textures->elements[i].rect = (Rectangle){  .x = elem_x, .y = elem_y, .width = box_w, .height = box_w};
        map_textures->elements[i].draw_rect = true;
        map_textures->elements[i].inner_texture = menu->t2d_background;
        map_textures->elements[i].texture_rect = GetSpriteRect(i, SQUARE_PIXEL_WIDTH, false, false);
        map_textures->elements[i].text = BACKGROUNDS_STRINGS[i];

        elem_x += box_w + box_xsp;
    }

    
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
}

void setup_menuscreen(GameState* state) {
    state->screen_size = (iVec2D){1200,1000};
    SetWindowSize(state->screen_size.x, state->screen_size.y);
    
    MenuGui* menu = malloc(sizeof(MenuGui));
    setup_menu(menu, state);
    // menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    // menu->text_elements = setup_menu(menu->t2d_background);
    
    state->screen_memory = menu;
}


void update_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    update_uicontext(&menu->gui);
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
    // UnloadTexture(menu->t2d_background);
    destroy_uicontext(&menu->gui);
    free(menu);
}