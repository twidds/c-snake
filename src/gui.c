#include <string.h> //memcpy
#include <assert.h>
#include "gui.h"
#include "arena.h"

#define ELEM_ARENA_STARTSIZE 10 * sizeof(UiElement)
#define BOXGROUP_ARENA_STARTSIZE 10 * sizeof(UiBoxGroup)
#define TEXT_ARENA_STARTSIZE 1024 * sizeof(char)
#define BOXGROUP_STARTSIZE 10
#define MAX_ELEMENTS 30
#define MAX_COMBOBOXES 15

/*  --------------------------------------------------------------------------------------- /
                                    UiTheme Functions
    --------------------------------------------------------------------------------------- */
Color get_theme_color_attr(UiTheme* theme, ElementState state, ElementType type, ElementColorAttr attr) {
    int index = state * ELEM_TYPE_COUNT * ELEM_COLOR_ATTR_COUNT + type * ELEM_COLOR_ATTR_COUNT + attr;
    return ((Color*)theme->elem_attributes[ELEM_COLOR_TYPE])[index];
}

float get_theme_float_attr(UiTheme* theme, ElementState state, ElementType type, ElementFloatAttr attr) {
    int index = state * ELEM_TYPE_COUNT * ELEM_FLOAT_ATTR_COUNT + type * ELEM_FLOAT_ATTR_COUNT + attr;
    return ((float*)theme->elem_attributes[ELEM_FLOAT_TYPE])[index];
}

int get_theme_int_attr(UiTheme* theme, ElementState state, ElementType type, ElementIntAttr attr) {
    int index = state * ELEM_TYPE_COUNT * ELEM_INT_ATTR_COUNT + type * ELEM_INT_ATTR_COUNT + attr;
    return ((int*)theme->elem_attributes[ELEM_INT_TYPE])[index];
}

void set_theme_color_attr(UiTheme* theme, ElementState state, ElementType type, ElementColorAttr attr, Color value) {
    int index = state * ELEM_TYPE_COUNT * ELEM_COLOR_ATTR_COUNT + type * ELEM_COLOR_ATTR_COUNT + attr;
    ((Color*)theme->elem_attributes[ELEM_COLOR_TYPE])[index] = value;
}

void set_theme_float_attr(UiTheme* theme, ElementState state, ElementType type, ElementFloatAttr attr, float value) {
    int index = state * ELEM_TYPE_COUNT * ELEM_FLOAT_ATTR_COUNT + type * ELEM_FLOAT_ATTR_COUNT + attr;
    ((float*)theme->elem_attributes[ELEM_FLOAT_TYPE])[index] = value;
}

void set_theme_int_attr(UiTheme* theme, ElementState state, ElementType type, ElementIntAttr attr, int value) {
    int index = state * ELEM_TYPE_COUNT * ELEM_INT_ATTR_COUNT + type * ELEM_INT_ATTR_COUNT + attr;
    ((int*)theme->elem_attributes[ELEM_INT_TYPE])[index] = value;
}

//WARNING: Exposes the pointer to the user, so they could just modify the default theme.
//TODO:: Need to think about better way to expose the default theme... maybe just pass the struct around
void uitheme_createdefault(UiContext* uictx) {
    uictx->default_theme = uitheme_create(uictx);
    int* int_attrs = uictx->default_theme->elem_attributes[ELEM_INT_TYPE];
    int* float_attrs = uictx->default_theme->elem_attributes[ELEM_FLOAT_TYPE];
    int* color_attrs = uictx->default_theme->elem_attributes[ELEM_COLOR_TYPE];
    
    for (int i = 0; i < ELEM_STATE_COUNT; i++) {
        set_theme_color_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, WHITE);
        set_theme_color_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_TEXT_COLOR, BLACK);
        set_theme_color_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_GLOW_COLOR, BLANK);
        set_theme_int_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_BORDER_THICKNESS, 2);
        set_theme_int_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_GLOW_THICKNESS, 0);
        set_theme_int_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);
        set_theme_float_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 12.0f);
        set_theme_float_attr(uictx->default_theme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SPACING, 1.0f);

        set_theme_color_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_INNER_COLOR, BLANK);
        set_theme_color_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_BORDER_COLOR, BLANK);
        set_theme_color_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_TEXT_COLOR, BLACK);
        set_theme_color_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_GLOW_COLOR, BLANK);
        set_theme_int_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_BORDER_THICKNESS, 0);
        set_theme_int_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_GLOW_THICKNESS, 0);
        set_theme_int_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);
        set_theme_float_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SIZE, 12.0f);
        set_theme_float_attr(uictx->default_theme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SPACING, 1.0f);
    }
    
    set_theme_color_attr(uictx->default_theme, ELEM_DEFAULT, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, BLACK);
    set_theme_color_attr(uictx->default_theme, ELEM_FOCUSED, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, GRAY);
    set_theme_color_attr(uictx->default_theme, ELEM_SELECTED, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, RED);

    uictx->default_theme->text_font = GetFontDefault();
}

UiTheme* uitheme_create(UiContext* uictx) {
    UiTheme* theme = arena_alloc(uictx->arena, sizeof(UiTheme));
    *theme = (UiTheme){0};
    theme->elem_attributes[ELEM_INT_TYPE] = arena_alloc(uictx->arena, sizeof(int) * ELEM_INT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    theme->elem_attributes[ELEM_FLOAT_TYPE] = arena_alloc(uictx->arena, sizeof(float) * ELEM_FLOAT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    theme->elem_attributes[ELEM_COLOR_TYPE] = arena_alloc(uictx->arena, sizeof(Color) * ELEM_COLOR_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    return theme;
}

UiTheme* uitheme_createcopy(UiContext* uictx, UiTheme* copyfrom) {
    UiTheme* theme = uitheme_create(uictx);
    
    theme->text_font = copyfrom->text_font;
    memcpy(theme->elem_attributes[ELEM_INT_TYPE], 
        copyfrom->elem_attributes[ELEM_INT_TYPE], 
        sizeof(int) * ELEM_INT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);

    memcpy(theme->elem_attributes[ELEM_FLOAT_TYPE], 
        copyfrom->elem_attributes[ELEM_FLOAT_TYPE], 
        sizeof(float) * ELEM_FLOAT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);

    memcpy(theme->elem_attributes[ELEM_COLOR_TYPE], 
        copyfrom->elem_attributes[ELEM_COLOR_TYPE], 
        sizeof(Color) * ELEM_COLOR_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
        
    return theme;
}

/*  --------------------------------------------------------------------------------------- /
                                    UiElement Functions
    --------------------------------------------------------------------------------------- */
UiElement* element_create(UiContext* uictx) {
    assert(uictx->elem_count <= MAX_ELEMENTS);

    UiElement* new_elem = &uictx->elements[uictx->elem_count];
    uictx->elem_count++;
    *new_elem = (UiElement){0};
    new_elem->theme = uictx->default_theme;
    new_elem->visible = true;
    return new_elem;
}

//Internal click action, calls user provided click action if relevant
static void elem_click(UiContext* ctx, UiElement* clicked) {
    clicked->state = ELEM_SELECTED;
    //Change selected in combobox (if applicable)
    if (clicked->parent_group) {
        if (clicked->parent_group->selected) {
            clicked->parent_group->selected->state = ELEM_DEFAULT;
        }
        clicked->parent_group->selected = clicked;
    }

    if (clicked->click_action) {
        clicked->click_action(ctx, clicked);
    }
}

static void draw_uielement(UiElement* element) {
    UiTheme* theme = element->theme;
    //TODO:: Re-implement glow color
    
    if (element->draw_rect) {
        Color border_color = get_theme_color_attr(theme, element->state, element->type, ELEM_COLOR_ATTR_BORDER_COLOR);
        Color inner_color = get_theme_color_attr(theme, element->state, element->type, ELEM_COLOR_ATTR_INNER_COLOR);
        int border_thickness = get_theme_int_attr(theme, element->state, element->type, ELEM_INT_ATTR_BORDER_THICKNESS);
        if (border_thickness) {
            Rectangle border_rect = (Rectangle){
                element->rect.x - border_thickness,
                element->rect.y - border_thickness,
                element->rect.width + border_thickness * 2,
                element->rect.height + border_thickness * 2
                };
            DrawRectangleRec(border_rect, border_color);
        }
        if (element->inner_texture.id) {
            DrawTexturePro(element->inner_texture, element->texture_rect, element->rect, (Vector2){0,0}, 0.0f, WHITE);
        } else {
            DrawRectangleRec(element->rect, inner_color);
        }

    }
    
    if (element->text) {
        float text_size = get_theme_float_attr(theme, element->state, element->type, ELEM_FLOAT_ATTR_TEXT_SIZE);
        float text_spacing = get_theme_float_attr(theme, element->state, element->type, ELEM_FLOAT_ATTR_TEXT_SPACING);
        Color text_color = get_theme_color_attr(theme, element->state, element->type, ELEM_COLOR_ATTR_TEXT_COLOR);
        TextAlignment text_align = get_theme_int_attr(theme, element->state, element->type, ELEM_INT_ATTR_TEXT_ALIGNMENT);
        
        Vector2 t_sz = MeasureTextEx(theme->text_font, element->text, text_size, text_spacing);
        Vector2 pos;
        switch(text_align) {
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
                pos.y = element->rect.y - text_spacing - t_sz.y;
                break;
            case ALIGN_BELOW:
                pos.x = element->rect.x + element->rect.width/2 - t_sz.x/2;
                pos.y = element->rect.y + text_spacing + element->rect.height;
                break;
        }
        sizeof(UiElement);

        DrawTextEx(theme->text_font, 
            element->text, 
            pos, 
            text_size, 
            text_spacing, 
            text_color);
    }
}

/*  --------------------------------------------------------------------------------------- /
                                    UiComboBox Functions
    --------------------------------------------------------------------------------------- */

//Creates a combobox with elem_count number of elements inside of it
UiComboBox* combobox_create(UiContext* uictx, size_t elem_count) {
    assert(uictx->uibox_count <= MAX_COMBOBOXES);
    UiComboBox* box = &uictx->comboboxes[uictx->uibox_count];
    *box = (UiComboBox){0};
    uictx->uibox_count++;

    for (int i = 0; i < elem_count; i++) {
        UiElement* elem = element_create(uictx);
        if (!box->elements) box->elements = elem;
        elem->parent_group = box;
    }
    box->count = elem_count;
    return box;
}

/*  --------------------------------------------------------------------------------------- /
                                Input Handling Functions
    --------------------------------------------------------------------------------------- */
bool is_inelementbounds(UiElement* elem, Vector2 pos) {
    return  elem->rect.x < pos.x &&
            elem->rect.y < pos.y &&
            elem->rect.x + elem->rect.width > pos.x &&
            elem->rect.y + elem->rect.height > pos.y;
}

/*  --------------------------------------------------------------------------------------- /
                                    UiContext Functions
    --------------------------------------------------------------------------------------- */
void setup_uicontext(UiContext* uictx) {
    *uictx = (UiContext){0}; //memset zero
    arena_init(&uictx->arena);
    uictx->elements = arena_alloc(uictx->arena, MAX_ELEMENTS * sizeof(UiElement));
    uictx->comboboxes = arena_alloc(uictx->arena, MAX_COMBOBOXES * sizeof(UiComboBox));
    uitheme_createdefault(uictx);
}

void destroy_uicontext(UiContext* uictx) {
    arena_destroy(&uictx->arena);
}

void update_uicontext(UiContext* uictx) {
    Vector2 mouse_pos = GetMousePosition();
    uictx->focused_elem = NULL;
    uictx->clicked_elem = NULL;

    //Handle mouse events
    for (int i = 0; i < uictx->elem_count; i++) {
        UiElement* elem = &uictx->elements[i];

        if (elem->state != ELEM_SELECTED) elem->state = ELEM_DEFAULT;
        if (is_inelementbounds(elem, mouse_pos) && elem->state != ELEM_SELECTED) {
            uictx->focused_elem = elem;
            elem->state = ELEM_FOCUSED;
        }
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (uictx->focused_elem) {
            uictx->pressed_elem = uictx->focused_elem;
        } else {
            uictx->pressed_elem = NULL;
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && uictx->pressed_elem && uictx->pressed_elem == uictx->focused_elem) {
        if (uictx->pressed_elem->state != ELEM_SELECTED) {
            elem_click(uictx, uictx->pressed_elem);
        }
    }
}

void draw_uicontext(UiContext* uictx) {
    //This doesn't work if elements are across multiple slabs
    for (int i = 0; i < uictx->elem_count; i++) {
        draw_uielement(&uictx->elements[i]);
    }
}
