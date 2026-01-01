#ifndef SNAKE_GUI_H
#define SNAKE_GUI_H

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Arena Arena;
typedef struct UiComboBox UiComboBox;
typedef struct UiElement UiElement;

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
    UiElement* focused_elem;
    UiElement* clicked_elem;
    UiElement* pressed_elem;
    UiElement* elements;
    UiComboBox* comboboxes;
    Arena* arena; //Memory locations must be stable
    
    int elem_count;
    int uibox_count;
} UiContext;

//Theme for rendering UiElement
typedef struct UiTheme {
    Font text_font;
    void* elem_attributes[ELEM_ATTR_TYPE_COUNT];
} UiTheme;

typedef struct UiElement{
    UiTheme* theme;
    UiComboBox* parent_group;
    const char* text;
    void* element_data; //can point to arbitrary context/data user wants as part of click action
    void(*click_action)(UiContext* ctx, struct UiElement* clicked_element);

    Texture2D inner_texture; //id == 0 means invalid texture
    Rectangle texture_rect;
    Rectangle rect;
    ElementState state;
    ElementType type;
    
    bool visible;
    bool draw_rect;
} UiElement;

typedef struct UiComboBox {
    UiElement* elements;
    UiElement* selected;
    size_t count;
} UiComboBox;

void setup_uicontext(UiContext* uictx);
void destroy_uicontext(UiContext* uictx);

UiElement* element_create(UiContext* uictx);
void element_settheme(UiTheme* uitheme);

UiTheme* uitheme_create(UiContext* uictx);
UiTheme* uitheme_createcopy(UiContext* uictx, UiTheme* copyfrom);
UiTheme* uitheme_getdefault(UiContext* uictx);

Color get_theme_color_attr(UiTheme* theme, ElementState state, ElementType type, ElementColorAttr attr);
float get_theme_float_attr(UiTheme* theme, ElementState state, ElementType type, ElementFloatAttr attr);
int get_theme_int_attr(UiTheme* theme, ElementState state, ElementType type, ElementIntAttr attr);
void set_theme_color_attr(UiTheme* theme, ElementState state, ElementType type, ElementColorAttr attr, Color value);
void set_theme_float_attr(UiTheme* theme, ElementState state, ElementType type, ElementFloatAttr attr, float value);
void set_theme_int_attr(UiTheme* theme, ElementState state, ElementType type, ElementIntAttr attr, int value);


UiComboBox* combobox_create(UiContext* uictx, size_t elem_count);

bool is_inelementbounds(UiElement* elem, Vector2 pos);
void update_uicontext(UiContext* uictx);
void draw_uicontext(UiContext* uictx);

#endif