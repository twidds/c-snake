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

typedef struct MenuGui{
    UiElement size_text;
    UiBoxGroup map_sizes;

    UiElement background_text;
    UiBoxGroup map_backgrounds;
    
    UiElement start_button;
    
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


//Returns array of UI elements representing the menu items
void setup_menu(MenuGui* menu) {
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");

    menu->map_backgrounds.boxes = malloc(sizeof(UiElement) * BACKGROUND_COUNT);
    menu->map_backgrounds.count = BACKGROUND_COUNT;

    menu->map_sizes.boxes = malloc(sizeof(UiElement) * MAP_SIZECOUNT);
    menu->map_sizes.count = MAP_SIZECOUNT;
    
    //Map sizes
    init_uielement(&menu->size_text);
    menu->size_text.draw_rect = false;
    menu->size_text.rect = (Rectangle){20, 20, 0, 0};
    menu->size_text.text = "Select grid size:";
    menu->size_text.text_align = ALIGN_LEFT;
    menu->size_text.text_size = 20.0f;

    menu->map_sizes.selected = MAP_MEDIUMSIZE;
    menu->map_sizes.hover_glow_color = GREEN;
    menu->map_sizes.hover_glow_thickness = 10;
    menu->map_sizes.selected_glow_color = ORANGE;
    menu->map_sizes.selected_glow_thickness = 15;
    
    init_uielement(&menu->map_sizes.boxes[MAP_SMALLSIZE]);
    menu->map_sizes.boxes[MAP_SMALLSIZE].draw_rect = true;
    menu->map_sizes.boxes[MAP_SMALLSIZE].border_thickness = 2;
    menu->map_sizes.boxes[MAP_SMALLSIZE].rect = (Rectangle){40, 40, 80, 30};
    menu->map_sizes.boxes[MAP_SMALLSIZE].text_align = ALIGN_CENTER;
    menu->map_sizes.boxes[MAP_SMALLSIZE].text = "8 x 8";
    menu->map_sizes.boxes[MAP_SMALLSIZE].text_size = 15.0f;
    
    init_uielement(&menu->map_sizes.boxes[MAP_MEDIUMSIZE]);
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].draw_rect = true;
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].border_thickness = 2;
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].rect = (Rectangle){170, 40, 80, 30};
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].text_align = ALIGN_CENTER;
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].text = "12 x 12";
    menu->map_sizes.boxes[MAP_MEDIUMSIZE].text_size = 15.0f;

    init_uielement(&menu->map_sizes.boxes[MAP_LARGESIZE]);
    menu->map_sizes.boxes[MAP_LARGESIZE].draw_rect = true;
    menu->map_sizes.boxes[MAP_LARGESIZE].border_thickness = 2;
    menu->map_sizes.boxes[MAP_LARGESIZE].rect = (Rectangle){300, 40, 80, 30};
    menu->map_sizes.boxes[MAP_LARGESIZE].text = "20 x 20";
    menu->map_sizes.boxes[MAP_LARGESIZE].text_align = ALIGN_CENTER;
    menu->map_sizes.boxes[MAP_LARGESIZE].text_size = 15.0f;

    //Map backgrounds
    init_uielement(&menu->background_text);
    menu->background_text.draw_rect = false;
    menu->background_text.rect = (Rectangle){20, 100, 0, 0};
    menu->background_text.text = "Select Background:";
    menu->background_text.text_align = ALIGN_LEFT;
    menu->background_text.text_size = 20.0f;
    menu->map_backgrounds.selected = BACKGROUND_DIRT;
    menu->map_backgrounds.hover_glow_color = BLUE;
    menu->map_backgrounds.hover_glow_thickness = 10;
    menu->map_backgrounds.selected_glow_color = RED;
    menu->map_backgrounds.selected_glow_thickness = 15;
    
    init_uielement(&menu->map_backgrounds.boxes[BACKGROUND_DIRT]);
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].draw_rect = true;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].border_thickness = 2;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].rect = (Rectangle){40, 120, 80, 80};
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].use_texture = true;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].inner_texture = menu->t2d_background;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].texture_rect = GetSpriteRect(BACKGROUND_DIRT, SQUARE_PIXEL_WIDTH, false, false);
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].text_align = ALIGN_BELOW;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].text_spacing = 2.0f;
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].text = "DIRT";
    menu->map_backgrounds.boxes[BACKGROUND_DIRT].text_size = 15.0f;

    init_uielement(&menu->map_backgrounds.boxes[BACKGROUND_WHITETILE]);
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].draw_rect = true;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].border_thickness = 2;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].rect = (Rectangle){170, 120, 80, 80};
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].use_texture = true;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].inner_texture = menu->t2d_background;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].texture_rect = GetSpriteRect(BACKGROUND_WHITETILE, SQUARE_PIXEL_WIDTH, false, false);
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text_align = ALIGN_BELOW;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text_spacing = 2.0f;
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text = "TILE";
    menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text_size = 15.0f;

    //Start button
    init_uielement(&menu->start_button);
    menu->start_button.draw_rect = true;
    menu->start_button.border_thickness = 2;
    menu->start_button.rect = (Rectangle){170, 240, 180, 40};
    menu->start_button.text = "START GAME";
    menu->start_button.text_align = ALIGN_CENTER;
    menu->start_button.text_size = 25.0f;
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
    state->screen_size = (iVec2D){800,800};
    SetWindowSize(state->screen_size.x, state->screen_size.y);
    
    MenuGui* menu = malloc(sizeof(MenuGui));
    setup_menu(menu);
    // menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    // menu->text_elements = setup_menu(menu->t2d_background);
    
    state->screen_memory = menu;
}

void update_menuscreen(GameState* state) {
    //check mouse positions and click states here
}

void draw_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    BeginDrawing();
    ClearBackground(SKYBLUE);
    
    draw_uielement(&menu->size_text);
    draw_uiboxgroup(&menu->map_sizes);
    
    draw_uielement(&menu->background_text);
    draw_uiboxgroup(&menu->map_backgrounds);
    
    draw_uielement(&menu->start_button);
    
    EndDrawing();
}

void unload_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    UnloadTexture(menu->t2d_background);
    free(menu->map_backgrounds.boxes);
    free(menu->map_sizes.boxes);
    free(menu);
}