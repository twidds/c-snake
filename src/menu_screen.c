#include "screens.h"
#include "gui.h"
#include <stdlib.h> //malloc, NULL

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

typedef struct {
    UiContext gui;

    UiElement* mouse_overelem;
    UiElement* mouse_downelem;
    UiElement* clicked_elem;

    UiElement* size_text;
    UiBoxGroup map_sizes;

    UiElement* background_text;
    UiBoxGroup map_backgrounds;

    UiElement* resolution_text;
    UiBoxGroup resolutions;
    
    UiElement* start_button;
    
    Texture2D t2d_background; //TODO:: remove?
} MenuGui; //TODO:: change to menu screen object


Rectangle GetSpriteRect(int sprite_index, int sprite_width, bool flip_x, bool flip_y) {
    Rectangle rect = {sprite_width*sprite_index, 0, sprite_width, sprite_width};
    rect.width = flip_x ? -1 * rect.width : rect.width;
    rect.height = flip_y ? -1 * rect.height : rect.height;
    return rect;
}

void elem_click(UiContext* uictx, ElementId id, void* click_context) {
    Texture2D* tex = click_context;
    Rectangle tex_rect = GetSpriteRect(BACKGROUND_DIRT, SQUARE_PIXEL_WIDTH, false, false);
    element_settexture(uictx, id, *tex, tex_rect);
    element_enabletexture(uictx, id);
}

void setup_menu(MenuGui* menu, GameState* state) {
    UiContext* uictx = &menu->gui;
    setup_uicontext(uictx);
    menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
    
    ElementId elem_id = element_create(uictx);
    element_setvisibility(uictx, elem_id, true);
    element_setrectanglevisibility(uictx, elem_id, true);
    element_setcolor(uictx, elem_id, WHITE);
    element_setborder(uictx, elem_id, BLACK, 2);
    element_setwidthheight(uictx, elem_id, 100, 20);
    element_setposition(uictx, elem_id, (iVec2D){20, 20});
    element_settext(uictx, elem_id, "TEST TEXT");
    element_setclickaction(uictx, elem_id, &menu->t2d_background, elem_click);
    
    // BoxGroupId res_group = boxgroup_create(uictx);
    
}


// void setup_menu_old(MenuGui* menu, GameState* state) {
//     setup_uicontext(&menu->gui);

//     menu->t2d_background = LoadTexture("assets/backgrounds_spritesheet.bmp");
//     menu->mouse_overelem = NULL;
//     menu->mouse_downelem = NULL;
//     menu->clicked_elem = NULL;
//     menu->elem_arena.buffer = NULL;

//     //Allocate elements
//     elemarena_alloc(&menu->elem_arena, 15);
    
//     menu->size_text = elemarena_addelems(&menu->elem_arena, 1);
//     menu->background_text = elemarena_addelems(&menu->elem_arena, 1);
//     menu->resolution_text = elemarena_addelems(&menu->elem_arena, 1);
//     menu->start_button = elemarena_addelems(&menu->elem_arena, 1);

//     menu->map_backgrounds.boxes = elemarena_addelems(&menu->elem_arena, BACKGROUND_COUNT);
//     menu->map_backgrounds.count = BACKGROUND_COUNT;

//     menu->map_sizes.boxes = elemarena_addelems(&menu->elem_arena, MAP_SIZECOUNT);
//     menu->map_sizes.count = MAP_SIZECOUNT;

//     menu->resolutions.boxes = elemarena_addelems(&menu->elem_arena, RES_COUNT);
//     menu->resolutions.count = RES_COUNT;

//     //Layout constants
//     const int text_lx = 20;

//     const int box_lx = 80;
//     const int box_w = 230;
//     const int box_h = 50;
//     const int box_xsp = 40;
    
//     const float text_fontsize = 30.0f;
//     const float box_fontsize = 40.0f;
//     const int ui_ygap = 40;
    
//     const int restxt_y = 40;
//     const int resbox_y = restxt_y + ui_ygap;
//     const int sztxt_y = resbox_y + box_h + ui_ygap*2;
//     const int mapbox_y = sztxt_y + ui_ygap;
//     const int backtxt_y = mapbox_y + box_h + ui_ygap*2;
//     const int backbox_y = backtxt_y + ui_ygap;
//     const int startbox_y = backbox_y + box_w + ui_ygap*2;
    
//     UiElement text_base;
//     init_uielement(&text_base);
//     text_base.draw_rect = false;
//     text_base.text_align = ALIGN_LEFT;
//     text_base.text_size = box_fontsize;

//     UiElement res_size_boxbase;
//     init_uielement(&res_size_boxbase);
//     res_size_boxbase.draw_rect = true;
//     res_size_boxbase.border_thickness = 2;
//     res_size_boxbase.text_align = ALIGN_CENTER;
//     res_size_boxbase.text_size = 40.0f;
    
//     UiElement back_boxbase;
//     init_uielement(&back_boxbase);
//     back_boxbase.draw_rect = true;
//     back_boxbase.border_thickness = 2;
//     back_boxbase.use_texture = true;
//     back_boxbase.inner_texture = menu->t2d_background;
//     back_boxbase.text_align = ALIGN_BELOW;
//     back_boxbase.text_spacing = 4.0f;
//     back_boxbase.text_size = 30.0f;

//     //Resolution sizes
//     *menu->resolution_text = text_base;
//     menu->resolution_text->rect = (Rectangle){text_lx, restxt_y, 0, 0};
//     menu->resolution_text->text = "Select game resolution";

//     menu->resolutions.boxes[RES_800x800] = res_size_boxbase;
//     menu->resolutions.boxes[RES_800x800].rect = (Rectangle){box_lx, resbox_y, box_w, box_h};
//     menu->resolutions.boxes[RES_800x800].text = "800 x 800";
    
//     menu->resolutions.boxes[RES_1200x1200] = res_size_boxbase;
//     menu->resolutions.boxes[RES_1200x1200].rect = (Rectangle){box_lx + (box_w + box_xsp), resbox_y, box_w, box_h};
//     menu->resolutions.boxes[RES_1200x1200].text = "1200 x 1200";
    
//     menu->resolutions.boxes[RES_1600x1600] = res_size_boxbase;
//     menu->resolutions.boxes[RES_1600x1600].rect = (Rectangle){box_lx + (box_w + box_xsp)*2, resbox_y, box_w, box_h};
//     menu->resolutions.boxes[RES_1600x1600].text = "1600 x 1600";

//     menu->resolutions.boxes[RES_2080x2080] = res_size_boxbase;
//     menu->resolutions.boxes[RES_2080x2080].rect = (Rectangle){box_lx + (box_w + box_xsp)*3, resbox_y, box_w, box_h};
//     menu->resolutions.boxes[RES_2080x2080].text = "2080 x 2080";
    
//     //Map sizes
//     *menu->size_text = text_base;
//     menu->size_text->rect = (Rectangle){text_lx, sztxt_y, 0, 0};
//     menu->size_text->text = "Select grid size:";

//     menu->map_sizes.selected = MAP_MEDIUMSIZE;
//     menu->map_sizes.hover_glow_color = GREEN;
//     menu->map_sizes.hover_glow_thickness = 10;
//     menu->map_sizes.selected_glow_color = ORANGE;
//     menu->map_sizes.selected_glow_thickness = 15;

//     menu->map_sizes.boxes[MAP_SMALLSIZE] = res_size_boxbase;
//     menu->map_sizes.boxes[MAP_SMALLSIZE].rect = (Rectangle){box_lx, mapbox_y, box_w, box_h};
//     menu->map_sizes.boxes[MAP_SMALLSIZE].text = "8 x 8";

//     menu->map_sizes.boxes[MAP_MEDIUMSIZE] = res_size_boxbase;
//     menu->map_sizes.boxes[MAP_MEDIUMSIZE].rect = (Rectangle){box_lx + (box_w + box_xsp), mapbox_y, box_w, box_h};
//     menu->map_sizes.boxes[MAP_MEDIUMSIZE].text = "12 x 12";

//     menu->map_sizes.boxes[MAP_LARGESIZE] = res_size_boxbase;
//     menu->map_sizes.boxes[MAP_LARGESIZE].rect = (Rectangle){box_lx + (box_w + box_xsp)*2, mapbox_y, box_w, box_h};
//     menu->map_sizes.boxes[MAP_LARGESIZE].text = "20 x 20";

//     //Map backgrounds
//     *menu->background_text = text_base;
//     menu->background_text->rect = (Rectangle){text_lx, backtxt_y, 0, 0};
//     menu->background_text->text = "Select Background:";
    
//     menu->map_backgrounds.selected = BACKGROUND_DIRT;
//     menu->map_backgrounds.hover_glow_color = BLUE;
//     menu->map_backgrounds.hover_glow_thickness = 10;
//     menu->map_backgrounds.selected_glow_color = RED;
//     menu->map_backgrounds.selected_glow_thickness = 15;
    
//     menu->map_backgrounds.boxes[BACKGROUND_DIRT] = back_boxbase;
//     menu->map_backgrounds.boxes[BACKGROUND_DIRT].rect = (Rectangle){box_lx, backbox_y, box_w, box_w};
//     menu->map_backgrounds.boxes[BACKGROUND_DIRT].texture_rect = GetSpriteRect(BACKGROUND_DIRT, SQUARE_PIXEL_WIDTH, false, false);
//     menu->map_backgrounds.boxes[BACKGROUND_DIRT].text = "DIRT";

//     menu->map_backgrounds.boxes[BACKGROUND_WHITETILE] = back_boxbase;
//     menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].rect = (Rectangle){box_lx + box_w + box_xsp, backbox_y, box_w, box_w};
//     menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].texture_rect = GetSpriteRect(BACKGROUND_WHITETILE, SQUARE_PIXEL_WIDTH, false, false);
//     menu->map_backgrounds.boxes[BACKGROUND_WHITETILE].text = "TILE";

//     //Start button
//     int startbox_w = 200;
//     int startbox_x = state->screen_size.x / 2 - startbox_w/2;
//     init_uielement(menu->start_button);
//     menu->start_button->draw_rect = true;
//     menu->start_button->border_thickness = 2;
//     menu->start_button->rect = (Rectangle){startbox_x, startbox_y, 200, 40};
//     menu->start_button->text = "START GAME";
//     menu->start_button->text_align = ALIGN_CENTER;
//     menu->start_button->text_size = 30.0f;
// }

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

    //if we click and release on a button, it's selected
    
    // IsMouseButtonPressed
    // IsMouseButtonReleased

}

void draw_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    BeginDrawing();
    ClearBackground(SKYBLUE);
    
    draw_uicontext(&menu->gui);
    // draw_uielement(menu->resolution_text);
    // draw_uiboxgroup(&menu->resolutions);

    // draw_uielement(menu->size_text);
    // draw_uiboxgroup(&menu->map_sizes);
    
    // draw_uielement(menu->background_text);
    // draw_uiboxgroup(&menu->map_backgrounds);
    
    // draw_uielement(menu->start_button);
    
    EndDrawing();
}

void unload_menuscreen(GameState* state) {
    MenuGui* menu = state->screen_memory;
    // UnloadTexture(menu->t2d_background);
    destroy_uicontext(&menu->gui);
    free(menu);
}