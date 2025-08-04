#ifndef SNAKE_GUI_H
#define SNAKE_GUI_H

#include "raylib.h"
#include "common.h" //iVec2D, bool
#include <stddef.h>

#define MAX_ELEMSPERBOXGROUP 20

typedef size_t ElementId;
typedef size_t BoxGroupId;

//TODO:: move to .c file
typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_BELOW,
    ALIGN_ABOVE
} TextAlignment;

typedef struct {
    size_t next_id;
    char* buffer;
    char* free;
    char* end;
} Arena;

typedef struct {
    ElementId mouse_over_elemID;
    ElementId mouse_clicked_elemID;
    ElementId mouse_down_elemID;
    Arena elem_arena;
    Arena bg_arena;

    // Arena text_arena;
} UiContext;

//TODO:: move to .c file
typedef struct UiElement{
    ElementId id;
    bool visible;

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
    
    const char* text;
    Color text_color;
    Font text_font;
    float text_size;
    float text_spacing;
    TextAlignment text_align;

    void(*click_action)(UiContext* ctx, ElementId clicked_element, void* click_context);
    void* click_context;
} UiElement;


//TODO:: move to .c file
typedef struct {
    BoxGroupId id;
    ElementId box_ids[MAX_ELEMSPERBOXGROUP];
    size_t count;
    
    ElementId selected;
    ElementId hovered;
    
    Color hover_glow_color;
    int hover_glow_thickness;

    Color selected_glow_color;
    int selected_glow_thickness;
} UiBoxGroup;






//UiContext functions
void setup_uicontext(UiContext* uictx);
void destroy_uicontext(UiContext* uictx);

// void elemarena_alloc(ElementArena* elem_arena, size_t size);
// void elemarena_dealloc(ElementArena* elem_arena);
// UiElement* elemarena_addelems(ElementArena* elem_arena, size_t count);

//Element setup functions
ElementId element_create(UiContext* uictx);
ElementId element_createbox(UiContext* uictx, Rectangle draw_rectangle);
// ElementId element_createbutton(UiContext* group, Rectangle draw_rect);

void element_settexture(UiContext* uictx, ElementId id, Texture2D texture, Rectangle texture_rectangle);
void element_enabletexture(UiContext* uictx, ElementId id);
void element_disabletexture(UiContext* uictx, ElementId id);
void element_setcolor(UiContext* uictx, ElementId id, Color color);
void element_setborder(UiContext* uictx, ElementId id, Color border_color, int border_thickness);
void element_setposition(UiContext* uictx, ElementId id, iVec2D position);
void element_setwidthheight(UiContext* uictx, ElementId id, int width, int height);
void element_setdrawrectangle(UiContext* uictx, ElementId id, Rectangle rectangle);
void element_setglow(UiContext* uictx, ElementId id, Color glow_color, int glow_thickness);
void element_settext(UiContext* uictx, ElementId id, const char* src_text); //Copies from src_text. text_len includes \0
void element_removeglow(UiContext* uictx, ElementId id);
void element_setvisibility(UiContext* uictx, ElementId id, bool is_visible);
void element_setrectanglevisibility(UiContext* uictx, ElementId id, bool is_visible);
void element_setclickaction(UiContext* uictx, ElementId id, void* click_context, void(*onclick)(UiContext* ctx, ElementId clicked_element, void* click_context));
void element_clonesettingsfromid(UiContext* uictx, ElementId src_id, ElementId dst_id);

BoxGroupId boxgroup_create(UiContext* uictx);
bool boxgroup_addelement(UiContext* uictx, BoxGroupId bg_id, ElementId elem_id);
void boxgroup_setglow_selected(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness);
void boxgroup_setglow_hover(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness);
bool boxgroup_containselement(UiBoxGroup* bg, ElementId elem_id);
void boxgroup_setselected(UiContext* uictx, UiBoxGroup* bg, ElementId elem_id);
void boxgroup_sethover(UiContext* uictx, UiBoxGroup* bg, ElementId elem_id);

//Iterate through elements:
    //Clicked:
        //If part of group: Set as selected <-- How do we tell?
            //Update appearance based on group settings
        //Call "clicked" action for user
            //They can update the data side

//Click on element in group:
    //Element becomes "selected"
    //Selected element properties are applied

//When start is clicked:
    //Get the selected map, size, resolution from UI elements? NO
    //We want UI elements to modify some "state" for what is selected
    //UI elements should manage their own appearance based on what's been configured
    //Set selected element "appearance" on the UI box group, not directly on the elements.


bool is_inelementbounds(UiElement* elem, Vector2 pos);

void update_uicontext(UiContext* uictx);
void draw_uicontext(UiContext* uictx);
// void draw_uielement(UiElement* element);
// void draw_uielement(UiContext* group, ElementId id); //TODO:: deprecate
// void draw_uiboxgroup(UiBoxGroup* group);
// void init_uielement(UiElement* element);



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