#ifndef SNAKE_GUI_H
#define SNAKE_GUI_H

#include "raylib.h"
#include "common.h" //iVec2D, bool
#include "arena.h"
#include <stddef.h>

#define MAX_ELEMSPERBOXGROUP 20

typedef size_t ElementId;
typedef size_t BoxGroupId;

//TODO:: move enum to .c file
typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_BELOW,
    ALIGN_ABOVE
} TextAlignment;

typedef enum {
    ELEM_DEFAULT,
    ELEM_FOCUSED,
    ELEM_SELECTED,
    ELEM_STATE_COUNT
} ElementState;

typedef enum {
    ELEM_TEXTBOX,
    ELEM_BUTTON,
    ELEM_TYPE_COUNT
} ElementType;

typedef enum {
    ELEM_INT_TYPE,
    ELEM_FLOAT_TYPE,
    ELEM_COLOR_TYPE,
    ELEM_ATTR_TYPE_COUNT
} ElementAttrType;

//Colors with alpha == 0 will not be drawn
typedef enum {
    ELEM_COLOR_ATTR_GLOW_COLOR,
    ELEM_COLOR_ATTR_INNER_COLOR,
    ELEM_COLOR_ATTR_BORDER_COLOR,
    ELEM_COLOR_ATTR_TEXT_COLOR,
    ELEM_COLOR_ATTR_COUNT
} ElementColorAttr;

typedef enum {
    ELEM_INT_ATTR_BORDER_THICKNESS, //Value of 0 means borderless
    ELEM_INT_ATTR_GLOW_THICKNESS, //Value of 0 means no glow
    ELEM_INT_ATTR_TEXT_ALIGNMENT,
    ELEM_INT_ATTR_COUNT
} ElementIntAttr;

typedef enum {
    ELEM_FLOAT_ATTR_TEXT_SIZE,
    ELEM_FLOAT_ATTR_TEXT_SPACING,
    ELEM_FLOAT_ATTR_COUNT
} ElementFloatAttr;

//UI stuff
typedef struct UiContext {
    ElementId mouse_over_elemID;
    ElementId mouse_clicked_elemID;
    ElementId mouse_down_elemID;
    Arena elem_arena; //Memory locations must be stable
    Arena uibox_arena; //Could be dynamic array instead
    Arena ui_arena; //Could be dynamic array instead
    
    int elem_count;
    int uibox_count;
    int mouse_clicked_elem_idx;
    int mouse_over_elem_idx;
} UiContext;

//Theme for rendering UiElement
typedef struct UiTheme {
    Font text_font;
    void* elem_attributes[ELEM_ATTR_TYPE_COUNT];
} UiTheme;

typedef struct UiElement{
    Texture2D inner_texture; //id == 0 means invalid texture
    Rectangle texture_rect;
    Rectangle rect;
    ElementState state;
    ElementType type;
    
    UiTheme* theme;
    const char* text;

    void(*focus_action)(UiContext* ctx, struct UiElement* focused_element);
    void(*click_action)(UiContext* ctx, struct UiElement* clicked_element);
    
    bool visible;
    bool draw_rect;
} UiElement;

typedef struct UiComboBox {
    UiElement* first;
    size_t count;
    // size_t selected_idx;
} UiComboBox;


//UiContext functions
void setup_uicontext(UiContext* uictx);
void destroy_uicontext(UiContext* uictx);

// void elemarena_alloc(ElementArena* elem_arena, size_t size);
// void elemarena_dealloc(ElementArena* elem_arena);
// UiElement* elemarena_addelems(ElementArena* elem_arena, size_t count);

//UiElement functions
UiElement* element_create(UiContext* uictx);
void element_settheme(UiTheme* uitheme);
// UiElement* element_createbox(UiContext* uictx, Rectangle draw_rectangle);
// ElementId element_createbutton(UiContext* group, Rectangle draw_rect);

//UiTheme functions
UiTheme* uitheme_create(UiContext* uictx);
UiTheme* uitheme_createcopy(UiContext* uictx, UiTheme* copyfrom);
UiTheme* uitheme_getdefault(UiContext* uictx);


// void element_settexture(UiContext* uictx, ElementId id, Texture2D texture, Rectangle texture_rectangle);
// void element_enabletexture(UiContext* uictx, ElementId id);
// void element_disabletexture(UiContext* uictx, ElementId id);
// void element_setcolor(UiContext* uictx, ElementId id, Color color);
// void element_setborder(UiContext* uictx, ElementId id, Color border_color, int border_thickness);
// void element_setposition(UiContext* uictx, ElementId id, iVec2D position);
// void element_setwidthheight(UiContext* uictx, ElementId id, int width, int height);
// void element_setdrawrectangle(UiContext* uictx, ElementId id, Rectangle rectangle);
// void element_setglow(UiContext* uictx, ElementId id, Color glow_color, int glow_thickness);
// void element_settext(UiContext* uictx, ElementId id, const char* src_text); //Copies from src_text. text_len includes \0
// void element_removeglow(UiContext* uictx, ElementId id);
// void element_setvisibility(UiContext* uictx, ElementId id, bool is_visible);
// void element_setrectanglevisibility(UiContext* uictx, ElementId id, bool is_visible);
// void element_setclickaction(UiContext* uictx, ElementId id, void* click_context, void(*onclick)(UiContext* ctx, ElementId clicked_element, void* click_context));
// void element_clonesettingsfromid(UiContext* uictx, ElementId src_id, ElementId dst_id);

UiComboBox* combobox_create(UiContext* uictx, size_t elem_count);
// void combobox_addelement(UiComboBox* box, UiElement* elem);

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
// void draw_uiboxgroup(UiComboBox* group);
// void init_uielement(UiElement* element);



//TODO:: Implement some api like this?
// void draw_element(UiElement element);
// UiElement create_element(bool draw_rectangle, bool );
//void element_setrectangle(Rectangle rectangle);
// void element_addtexture(UiElement elem, Texture2D texture, Rectangle texture_rectangle);
// void element_addborder(int thickness, Color color);
// void element_addglow(bool enable_glow, int glow_thickness, Color glow_color);
// void element_removeglow(UiElement);

// UiComboBox create_boxgroup(int count);
// void boxgroup_setselected(UiElement element);
// UiElement boxgroup_getselected();
// UiElement boxgroup_getelement(int id);
// void draw_boxgroup(UiComboBox group);

#endif