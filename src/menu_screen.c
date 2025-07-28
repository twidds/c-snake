#include "screens.h"
#include "common.h"
#include "raylib.h"
#include <stdlib.h> //malloc

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

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_BELOW,
    ALIGN_ABOVE
} TextAlignment;

typedef enum {
    MAP_SMALLSIZE,
    MAP_MEDIUMSIZE,
    MAP_LARGESIZE,
    MAP_SIZECOUNT
} MapSize;

typedef enum {
    RES_800x800,
    RES_1200x1200,
    RES_1600x1600,
    RES_2080x2080,
    RES_COUNT
} ScreenRes;

// typedef enum {
//     MENU_DIRTBACKGROUND,
//     MENU_WHITEBACKGROUND,
//     MENU_BACKGROUNDCOUNT
// } MapBackground;


typedef struct UiElement{
    bool draw_rect; //may be false for text only
    bool use_texture;
    Rectangle rect;
    Color inner_color;
    int border_thickness;
    Color border_color;
    Rectangle texture_rect;
    Texture2D inner_texture;

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

typedef struct {
    UiElement* boxes;
    int count;
    int selected; //none is -1
    int hovering; //none is -1
    Color hover_glow_color;
    int hover_glow_thickness;
    Color selected_glow_color;
    int selected_glow_thickness;
} UiBoxGroup;

typedef struct {
    int count;
    int max;
    UiElement* buffer;
} ElementArena;

typedef struct MenuGui{
    UiElement* mouse_selected;
    ElementArena elem_arena;
    UiElement* size_text;
    UiBoxGroup map_sizes;

    UiElement* background_text;
    UiBoxGroup map_backgrounds;

    UiElement* resolution_text;
    UiBoxGroup resolutions;
    
    UiElement* start_button;
    
    Texture2D t2d_background; //TODO:: remove?
} MenuGui;


// typedef struct MenuScene{
//     Camera2D camera;
//     MenuGui menu_gui;
//     Texture2D* textures;
// } MenuScene;

Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y);
MenuGui*  setup_snakemenu(Texture2D* textures); //Tightly tied to game logic....
void draw_menu(MenuGui* menu);
void init_uielement(UiElement* element);
void draw_uielement(UiElement* element); //Necessary to export?


Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    rect.width = flip_x ? -1 * rect.width : rect.width;
    rect.height = flip_y ? -1 * rect.height : rect.height;
    return rect;
}

//set default values for UI element (zeroing where appropriate)
//DOES NOT CONSTRUCT ELEMENT
//Relies on GetFontDefault from raylib
void init_uielement(UiElement* element) {
    element->draw_rect = true;
    element->use_texture = false;
    element->rect = (Rectangle){0};
    element->inner_color = WHITE;
    element->border_thickness = 0;
    element->border_color = BLACK;
    element->texture_rect = (Rectangle){0};
    element->inner_texture = (Texture2D){0};


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

void elemarena_alloc(ElementArena* arena, int size) {
    if (arena->buffer) {free(arena->buffer);}
    arena->buffer = malloc(sizeof(UiElement) * size);
    arena->count = 0;
    arena->max = size;
}

void elemarena_dealloc(ElementArena* arena) {
    if (arena->buffer) {
        free(arena->buffer);
    }
    arena->count = 0;
    arena->max = 0;
}

//Adds a number of elements to the GUI, returns pointer to first in the new range of elements.
//If there's not enough room, arena buffer is reallocated.
UiElement* elemarena_addelems(ElementArena* arena, int count) {
    if (count + arena->count > arena->max) {
        int newmax = (count + arena->count) * 1.5;
        UiElement* newbuf = malloc(sizeof(UiElement) * newmax);
        for (int i = 0; i < arena->count; i++) {
            newbuf[i] = arena->buffer[i];
        }
        free(arena->buffer);
        arena->buffer = newbuf;
        arena->max = newmax;
    }

    UiElement* start = &arena->buffer[arena->count];
    arena->count += count;
    return start;
}


//Returns array of UI elements representing the menu items
void setup_menu(MenuGui* menu, GameState* state) {
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");

    //Allocate elements
    elemarena_alloc(&menu->elem_arena, 15);
    
    menu->size_text = elemarena_addelems(&menu->elem_arena, 1);
    menu->background_text = elemarena_addelems(&menu->elem_arena, 1);
    menu->resolution_text = elemarena_addelems(&menu->elem_arena, 1);
    menu->start_button = elemarena_addelems(&menu->elem_arena, 1);

    menu->map_backgrounds.boxes = elemarena_addelems(&menu->elem_arena, BACKGROUND_COUNT);
    menu->map_backgrounds.count = BACKGROUND_COUNT;

    menu->map_sizes.boxes = elemarena_addelems(&menu->elem_arena, MAP_SIZECOUNT);
    menu->map_sizes.count = MAP_SIZECOUNT;

    menu->resolutions.boxes = elemarena_addelems(&menu->elem_arena, RES_COUNT);
    menu->resolutions.count = RES_COUNT;

    //Layout constants
    const int text_lx = 20;

    const int box_lx = 80;
    const int box_w = 230;
    const int box_h = 50;
    const int box_xsp = 40;
    
    const float text_fontsize = 30.0f;
    const float box_fontsize = 40.0f;
    const int ui_ygap = 40;
    
    const int restxt_y = 40;
    const int resbox_y = restxt_y + ui_ygap;
    const int sztxt_y = resbox_y + box_h + ui_ygap*2;
    const int mapbox_y = sztxt_y + ui_ygap;
    const int backtxt_y = mapbox_y + box_h + ui_ygap*2;
    const int backbox_y = backtxt_y + ui_ygap;
    const int startbox_y = backbox_y + box_w + ui_ygap*2;
    
    UiElement text_base;
    init_uielement(&text_base);
    text_base.draw_rect = false;
    text_base.text_align = ALIGN_LEFT;
    text_base.text_size = box_fontsize;

    UiElement res_size_boxbase;
    init_uielement(&res_size_boxbase);
    res_size_boxbase.draw_rect = true;
    res_size_boxbase.border_thickness = 2;
    res_size_boxbase.text_align = ALIGN_CENTER;
    res_size_boxbase.text_size = 40.0f;
    
    UiElement back_boxbase;
    init_uielement(&back_boxbase);
    back_boxbase.draw_rect = true;
    back_boxbase.border_thickness = 2;
    back_boxbase.use_texture = true;
    back_boxbase.inner_texture = menu->t2d_background;
    back_boxbase.text_align = ALIGN_BELOW;
    back_boxbase.text_spacing = 4.0f;
    back_boxbase.text_size = 30.0f;

    //Resolution sizes
    *menu->resolution_text = text_base;
    menu->resolution_text->rect = (Rectangle){text_lx, restxt_y, 0, 0};
    menu->resolution_text->text = "Select game resolution";

    menu->resolutions.boxes[RES_800x800] = res_size_boxbase;
    menu->resolutions.boxes[RES_800x800].rect = (Rectangle){box_lx, resbox_y, box_w, box_h};
    menu->resolutions.boxes[RES_800x800].text = "800 x 800";
    
    menu->resolutions.boxes[RES_1200x1200] = res_size_boxbase;
    menu->resolutions.boxes[RES_1200x1200].rect = (Rectangle){box_lx + (box_w + box_xsp), resbox_y, box_w, box_h};
    menu->resolutions.boxes[RES_1200x1200].text = "1200 x 1200";
    
    menu->resolutions.boxes[RES_1600x1600] = res_size_boxbase;
    menu->resolutions.boxes[RES_1600x1600].rect = (Rectangle){box_lx + (box_w + box_xsp)*2, resbox_y, box_w, box_h};
    menu->resolutions.boxes[RES_1600x1600].text = "1600 x 1600";

    menu->resolutions.boxes[RES_2080x2080] = res_size_boxbase;
    menu->resolutions.boxes[RES_2080x2080].rect = (Rectangle){box_lx + (box_w + box_xsp)*3, resbox_y, box_w, box_h};
    menu->resolutions.boxes[RES_2080x2080].text = "2080 x 2080";
    
    //Map sizes
    *menu->size_text = text_base;
    menu->size_text->rect = (Rectangle){text_lx, sztxt_y, 0, 0};
    menu->size_text->text = "Select grid size:";

    menu->map_sizes.selected = MAP_MEDIUMSIZE;
    menu->map_sizes.hover_glow_color = GREEN;
    menu->map_sizes.hover_glow_thickness = 10;
    menu->map_sizes.selected_glow_color = ORANGE;
    menu->map_sizes.selected_glow_thickness = 15;

    menu->map_sizes.boxes[MAP_SMALLSIZE] = res_size_boxbase;
    menu->map_sizes.boxes[MAP_SMALLSIZE].rect = (Rectangle){box_lx, mapbox_y, box_w, box_h};
    menu->map_sizes.boxes[MAP_SMALLSIZE].text = "8 x 8";

    menu->map_sizes.boxes[MAP_MEDIUMSIZE] = res_size_boxbase;
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].rect = (Rectangle){box_lx + (box_w + box_xsp), mapbox_y, box_w, box_h};
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].text = "12 x 12";

    menu->map_sizes.boxes[MAP_LARGESIZE] = res_size_boxbase;
    menu->map_sizes.boxes[MAP_LARGESIZE].rect = (Rectangle){box_lx + (box_w + box_xsp)*2, mapbox_y, box_w, box_h};
    menu->map_sizes.boxes[MAP_LARGESIZE].text = "20 x 20";

    //Map backgrounds
    *menu->background_text = text_base;
    menu->background_text->rect = (Rectangle){text_lx, backtxt_y, 0, 0};
    menu->background_text->text = "Select Background:";
    
    menu->map_backgrounds.selected = BACKGROUND_DIRT;
    menu->map_backgrounds.hover_glow_color = BLUE;
    menu->map_backgrounds.hover_glow_thickness = 10;
    menu->map_backgrounds.selected_glow_color = RED;
    menu->map_backgrounds.selected_glow_thickness = 15;
    
    menu->map_backgrounds.boxes[BACKGROUND_DIRT] = back_boxbase;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].rect = (Rectangle){box_lx, backbox_y, box_w, box_w};
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].texture_rect = GetSpriteRect(BACKGROUND_DIRT, SQUARE_PIXEL_WIDTH, false, false);
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].text = "DIRT";

    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE] = back_boxbase;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].rect = (Rectangle){box_lx + box_w + box_xsp, backbox_y, box_w, box_w};
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].texture_rect = GetSpriteRect(BACKGROUND_WHITETILE, SQUARE_PIXEL_WIDTH, false, false);
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text = "TILE";

    //Start button
    int startbox_w = 200;
    int startbox_x = state->screen_size.x / 2 - startbox_w/2;
    init_uielement(menu->start_button);
    menu->start_button->draw_rect = true;
    menu->start_button->border_thickness = 2;
    menu->start_button->rect = (Rectangle){startbox_x, startbox_y, 200, 40};
    menu->start_button->text = "START GAME";
    menu->start_button->text_align = ALIGN_CENTER;
    menu->start_button->text_size = 30.0f;
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
        if (element->use_texture) {
            DrawTexturePro(element->inner_texture, element->texture_rect, element->rect, (Vector2){0,0}, 0.0f, WHITE);
        } else {
            DrawRectangleRec(element->rect, element->inner_color);
        }

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
            case ALIGN_ABOVE:
                pos.x = element->rect.x + element->rect.width/2 - t_sz.x/2;
                pos.y = element->rect.y - element->text_spacing - t_sz.y;
                break;
            case ALIGN_BELOW:
                pos.x = element->rect.x + element->rect.width/2 - t_sz.x/2;
                pos.y = element->rect.y + element->text_spacing + element->rect.height;
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


void draw_uiboxgroup(UiBoxGroup* group) {
    //handle glow effect based on selected
    //handle glow effect based on hover
    for (int i = 0; i < group->count; i++) {
        draw_uielement(&group->boxes[i]);
    }
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
    //check mouse positions and click states here
    Vector2 mouse_pos = GetMousePosition();
    //if we click and release on a button, it's selected
    
    // IsMouseButtonPressed
    // IsMouseButtonReleased

}

void draw_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    BeginDrawing();
    ClearBackground(SKYBLUE);
    
    draw_uielement(menu->resolution_text);
    draw_uiboxgroup(&menu->resolutions);

    draw_uielement(menu->size_text);
    draw_uiboxgroup(&menu->map_sizes);
    
    draw_uielement(menu->background_text);
    draw_uiboxgroup(&menu->map_backgrounds);
    
    draw_uielement(menu->start_button);
    
    EndDrawing();
}

void unload_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    UnloadTexture(menu->t2d_background);
    elemarena_dealloc(&menu->elem_arena);
    free(menu);
}