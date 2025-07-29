#ifndef SNAKE_GUI_H
#define SNAKE_GUI_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_BELOW,
    ALIGN_ABOVE
} TextAlignment;



typedef struct UiElement{
    int id;

    bool draw_rect;
    Rectangle rect;
    Color inner_color;
    
    bool use_texture;
    Rectangle texture_rect;
    Texture2D inner_texture;

    int border_thickness;
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


void elemarena_alloc(ElementArena* arena, int size);
void elemarena_dealloc(ElementArena* arena);
UiElement* elemarena_addelems(ElementArena* arena, int count);
void draw_uiboxgroup(UiBoxGroup* group);
void draw_uielement(UiElement* element);
bool is_inelementbounds(UiElement* elem, Vector2 pos);
void init_uielement(UiElement* element);


//TODO:: Implement some api like this?
// void draw_element(UiElement element);
// UiElement create_element(bool draw_rectangle, bool );
//void element_setrectangle(Rectangle rectangle);
// void element_addtexture(UiElement elem, Texture2D texture, Rectangle texture_rectangle);
// void element_addborder(int thickness, Color color);
// void element_addglow(bool enable_glow, int glow_thickness, Color glow_color);
// void element_removeglow(UiElement);

// UiBoxGroup create_boxgroup(int count);
// void boxgroup_setselected(UiElement element);
// UiElement boxgroup_getselected();
// UiElement boxgroup_getelement(int id);
// void draw_boxgroup(UiBoxGroup group);

#endif