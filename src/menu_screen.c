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
    BACKGROUND_BLACK_BORDER,
    BACKGROUND_DIRT,
    BACKGROUND_SPRITE_COUNT
} BackgroundSpriteIdx;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_BELOW,
    ALIGN_ABOVE
} TextAlignment;

typedef enum{
    MENU_SIZETEXT,
    MENU_SMALLSIZE,
    MENU_MEDIUMSIZE,
    MENU_LARGESIZE,
    MENU_BACKGROUNDTEXT,
    MENU_DIRTBACKGROUND,
    MENU_WHITEBACKGROUND,
    MENU_STARTGAME,
    MENU_ITEMCOUNT
} MenuElement;


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

    int ui_group;
} UiElement;

typedef struct MenuUi{
    int n_elements;
    UiElement* elements;
    MenuElement selected_size;
    MenuElement selected_background;
    Texture2D t2d_background;
    // int* group_ids; //use for toggling selections
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
    
    element->ui_group = 0;
}

//Returns array of UI elements representing the menu items
UiElement*  setup_menu(Texture2D background) {
    UiElement* items_out = malloc(sizeof(UiElement) * MENU_ITEMCOUNT);
    for (int i = 0; i < MENU_ITEMCOUNT; i++) {
        init_uielement(&items_out[i]);
    }
    items_out[MENU_SIZETEXT].draw_rect = false;
    items_out[MENU_SIZETEXT].rect = (Rectangle){20, 20, 0, 0};
    items_out[MENU_SIZETEXT].text = "Select grid size:";
    items_out[MENU_SIZETEXT].text_align = ALIGN_LEFT;
    items_out[MENU_SIZETEXT].text_size = 20.0f;

    items_out[MENU_SMALLSIZE].draw_rect = true;
    items_out[MENU_SMALLSIZE].border_thickness = 2;
    items_out[MENU_SMALLSIZE].rect = (Rectangle){40, 40, 80, 30};
    items_out[MENU_SMALLSIZE].text_align = ALIGN_CENTER;
    items_out[MENU_SMALLSIZE].text = "8 x 8";
    items_out[MENU_SMALLSIZE].text_size = 15.0f;
    
    items_out[MENU_MEDIUMSIZE].draw_rect = true;
    items_out[MENU_MEDIUMSIZE].border_thickness = 2;
    items_out[MENU_MEDIUMSIZE].rect = (Rectangle){170, 40, 80, 30};
    items_out[MENU_MEDIUMSIZE].text_align = ALIGN_CENTER;
    items_out[MENU_MEDIUMSIZE].text = "12 x 12";
    items_out[MENU_MEDIUMSIZE].text_size = 15.0f;

    items_out[MENU_LARGESIZE].draw_rect = true;
    items_out[MENU_LARGESIZE].border_thickness = 2;
    items_out[MENU_LARGESIZE].rect = (Rectangle){300, 40, 80, 30};
    items_out[MENU_LARGESIZE].text = "20 x 20";
    items_out[MENU_LARGESIZE].text_align = ALIGN_CENTER;
    items_out[MENU_LARGESIZE].text_size = 15.0f;

    items_out[MENU_BACKGROUNDTEXT].draw_rect = false;
    items_out[MENU_BACKGROUNDTEXT].rect = (Rectangle){20, 100, 0, 0};
    items_out[MENU_BACKGROUNDTEXT].text = "Select Background:";
    items_out[MENU_BACKGROUNDTEXT].text_align = ALIGN_LEFT;
    items_out[MENU_BACKGROUNDTEXT].text_size = 20.0f;

    items_out[MENU_DIRTBACKGROUND].draw_rect = true;
    items_out[MENU_DIRTBACKGROUND].border_thickness = 2;
    items_out[MENU_DIRTBACKGROUND].rect = (Rectangle){40, 120, 80, 80};
    items_out[MENU_DIRTBACKGROUND].use_texture = true;
    items_out[MENU_DIRTBACKGROUND].inner_texture = background;
    items_out[MENU_DIRTBACKGROUND].texture_rect = GetSpriteRect(BACKGROUND_DIRT, SQUARE_PIXEL_WIDTH, false, false);
    items_out[MENU_DIRTBACKGROUND].text_align = ALIGN_BELOW;
    items_out[MENU_DIRTBACKGROUND].text_spacing = 2.0f;
    items_out[MENU_DIRTBACKGROUND].text = "DIRT";
    items_out[MENU_DIRTBACKGROUND].text_size = 15.0f;

    items_out[MENU_WHITEBACKGROUND].draw_rect = true;
    items_out[MENU_WHITEBACKGROUND].border_thickness = 2;
    items_out[MENU_WHITEBACKGROUND].rect = (Rectangle){170, 120, 80, 80};
    items_out[MENU_WHITEBACKGROUND].use_texture = true;
    items_out[MENU_WHITEBACKGROUND].inner_texture = background;
    items_out[MENU_WHITEBACKGROUND].texture_rect = GetSpriteRect(BACKGROUND_BLACK_BORDER, SQUARE_PIXEL_WIDTH, false, false);
    items_out[MENU_WHITEBACKGROUND].text_align = ALIGN_BELOW;
    items_out[MENU_WHITEBACKGROUND].text_spacing = 2.0f;
    items_out[MENU_WHITEBACKGROUND].text = "TILE";
    items_out[MENU_WHITEBACKGROUND].text_size = 15.0f;

    items_out[MENU_STARTGAME].draw_rect = true;
    items_out[MENU_STARTGAME].border_thickness = 2;
    items_out[MENU_STARTGAME].rect = (Rectangle){170, 240, 180, 40};
    items_out[MENU_STARTGAME].text = "START GAME";
    items_out[MENU_STARTGAME].text_align = ALIGN_CENTER;
    items_out[MENU_STARTGAME].text_size = 25.0f;
    
    return items_out;
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



void setup_menuscreen(GameState* state) {
    state->screen_size = (iVec2D){800,800};
    SetWindowSize(state->screen_size.x, state->screen_size.y);
    
    MenuGui* menu = malloc(sizeof(MenuGui));
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    menu->elements = setup_menu(menu->t2d_background);
    
    state->screen_memory = menu;
}

void update_menuscreen(GameState* state) {
    //check mouse positions and click states here
}

void draw_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    BeginDrawing();
    ClearBackground(SKYBLUE);
    for (int i = 0; i < MENU_ITEMCOUNT; i++) {
        draw_uielement(&menu->elements[i]);
    }
    EndDrawing();
}

void unload_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    UnloadTexture(menu->t2d_background);
    free(menu->elements);
    free(menu);
}