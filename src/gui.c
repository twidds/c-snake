#include "gui.h"
#include <string.h> //memcpy

#define ELEM_ARENA_STARTSIZE 10 * sizeof(UiElement)
#define BOXGROUP_ARENA_STARTSIZE 10 * sizeof(UiBoxGroup)
#define TEXT_ARENA_STARTSIZE 1024 * sizeof(char)
#define BOXGROUP_STARTSIZE 10
static UiTheme* defaultUiTheme = NULL;

/*  --------------------------------------------------------------------------------------- /
                                    UiGroup Functions
    --------------------------------------------------------------------------------------- */
void setup_uicontext(UiContext* uictx) {
    *uictx = (UiContext){0}; //memset zero
    arena_init(&uictx->ui_arena);
    arena_init(&uictx->elem_arena);
    arena_init(&uictx->uibox_arena);
}

void destroy_uicontext(UiContext* uictx) {
    arena_destroy(&uictx->ui_arena);
}


/*  --------------------------------------------------------------------------------------- /
                                    UiElement Functions
    --------------------------------------------------------------------------------------- */
UiElement* element_create(UiContext* uictx) {
    //NOTE: consecutive calls to arena_alloc are not guaranteed to give consecutive
    //      UiElement pointers unless UiElement struct stays 8-byte padded AND 
    //      all elements fit in one arena
    return arena_alloc(&uictx->elem_arena, sizeof(UiElement));
    uictx->elem_count++;
}


/*  --------------------------------------------------------------------------------------- /
                                    UiTheme Functions
    --------------------------------------------------------------------------------------- */

void set_theme_color_attr(UiTheme* theme, ElementState state, ElementType type, ElementColorAttr attr, Color value) {
    ((Color*)theme->elem_attributes[ELEM_COLOR_TYPE])[state * type] = value;
}

void set_theme_float_attr(UiTheme* theme, ElementState state, ElementType type, ElementFloatAttr attr, float value) {
    ((float*)theme->elem_attributes[ELEM_FLOAT_TYPE])[state * type] = value;
}

void set_theme_int_attr(UiTheme* theme, ElementState state, ElementType type, ElementIntAttr attr, int value) {
    ((int*)theme->elem_attributes[ELEM_INT_TYPE])[state * type] = value;
}

//WARNING: Exposes the pointer to the user, so they could just modify the default theme.
//TODO:: Need to think about better way to expose the default theme... maybe just pass the struct around
UiTheme* uitheme_getdefault(UiContext* uictx) {
    if (!defaultUiTheme) {
        defaultUiTheme = uitheme_create(uictx);
        
        for (int i = 0; i < ELEM_STATE_COUNT; i++) {
            set_theme_color_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_INNER_COLOR, WHITE);
            set_theme_color_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_TEXT_COLOR, BLACK);
            set_theme_color_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_COLOR_ATTR_GLOW_COLOR, BLANK);
            set_theme_int_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_INT_ATTR_BORDER_THICKNESS, 2);
            set_theme_int_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_INT_ATTR_GLOW_THICKNESS, 0);
            set_theme_int_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);
            set_theme_float_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SIZE, 12.0f);
            set_theme_float_attr(defaultUiTheme, i, ELEM_BUTTON, ELEM_FLOAT_ATTR_TEXT_SPACING, 1.0f);

            set_theme_color_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_INNER_COLOR, BLANK);
            set_theme_color_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_BORDER_COLOR, BLANK);
            set_theme_color_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_TEXT_COLOR, BLACK);
            set_theme_color_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_COLOR_ATTR_GLOW_COLOR, BLANK);
            set_theme_int_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_BORDER_THICKNESS, 0);
            set_theme_int_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_GLOW_THICKNESS, 0);
            set_theme_int_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_INT_ATTR_TEXT_ALIGNMENT, ALIGN_CENTER);
            set_theme_float_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SIZE, 12.0f);
            set_theme_float_attr(defaultUiTheme, i, ELEM_TEXTBOX, ELEM_FLOAT_ATTR_TEXT_SPACING, 1.0f);
        }
        
        set_theme_color_attr(defaultUiTheme, ELEM_DEFAULT, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, BLACK);
        set_theme_color_attr(defaultUiTheme, ELEM_FOCUSED, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, GRAY);
        set_theme_color_attr(defaultUiTheme, ELEM_SELECTED, ELEM_BUTTON, ELEM_COLOR_ATTR_BORDER_COLOR, RED);

        defaultUiTheme->text_font = GetFontDefault();
    }
    return defaultUiTheme;
}

const size_t RAW_SIZE = sizeof(int) * 3 + sizeof(Texture2D) + sizeof(void*) * 2;

UiTheme* uitheme_create(UiContext* uictx) {
    UiTheme* theme = arena_alloc(&uictx->ui_arena, sizeof(UiTheme));
    *theme = (UiTheme){0};
    theme->elem_attributes[ELEM_INT_TYPE] = arena_alloc(&uictx->ui_arena, sizeof(int) * ELEM_INT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    theme->elem_attributes[ELEM_FLOAT_TYPE] = arena_alloc(&uictx->ui_arena, sizeof(float) * ELEM_FLOAT_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    theme->elem_attributes[ELEM_COLOR_TYPE] = arena_alloc(&uictx->ui_arena, sizeof(Color) * ELEM_COLOR_ATTR_COUNT * ELEM_TYPE_COUNT * ELEM_STATE_COUNT);
    return theme;
}

UiTheme* uitheme_createcopy(UiContext* uictx, UiTheme* copyfrom) {
    UiTheme* theme = arena_alloc(&uictx->ui_arena, sizeof(UiTheme));
    memcpy(theme, copyfrom, sizeof(UiTheme));
    return theme;
}


// ElementId element_create(UiContext* uictx) {
//     UiElement* elem = arena_alloc(&uictx->elem_arena, sizeof(UiElement), 1);
//     while (!elem){
//         arena_grow(&uictx->elem_arena, 1.5);
//         elem = arena_alloc(&uictx->elem_arena, sizeof(UiElement), 1);
//     }
//     *elem = (UiElement){0};
//     elem->id = uictx->elem_arena.next_id;
//     uictx->elem_arena.next_id++;
//     elem->visible = true;
//     return elem->id;
// }


// ElementId element_createbox(UiContext* uictx, Rectangle draw_rectangle) {
//     ElementId id = element_create(uictx);
//     element_setdrawrectangle(uictx, id, draw_rectangle);
//     return id;
// }

// void element_settexture(UiContext* uictx, ElementId id, Texture2D texture, Rectangle texture_rectangle) {
//     UiElement* elem = get_elementbyid(uictx, id);
//     elem->inner_texture = texture;
//     elem->texture_rect = texture_rectangle;
// }

// void element_enabletexture(UiContext* uictx, ElementId id) {
//     get_elementbyid(uictx, id)->use_texture = true;
// }

// void element_disabletexture(UiContext* uictx, ElementId id) {
//     get_elementbyid(uictx, id)->use_texture = false;
// }

// void element_setcolor(UiContext* uictx, ElementId id, Color color){
//     get_elementbyid(uictx, id)->inner_color = color;
// }

// void element_setborder(UiContext* uictx, ElementId id, Color border_color, int border_thickness) {
//     UiElement* elem = get_elementbyid(uictx, id);
//     elem->border_color = border_color;
//     elem->border_thickness = border_thickness;
// }

// void element_setposition(UiContext* uictx, ElementId id, iVec2D position) {
//     UiElement* elem = get_elementbyid(uictx, id);
//     elem->rect.x = position.x;
//     elem->rect.y = position.y;
// }

// void element_setwidthheight(UiContext* uictx, ElementId id, int width, int height) {
//     UiElement* elem = get_elementbyid(uictx, id);
//     elem->rect.width = width;
//     elem->rect.height = height;
// }

// void element_setdrawrectangle(UiContext* uictx, ElementId id, Rectangle rectangle) {
//     get_elementbyid(uictx, id)->rect = rectangle;
// }

// void element_setglow(UiContext* uictx, ElementId id, Color glow_color, int glow_thickness) {
//     UiElement* elem = get_elementbyid(uictx, id);
//     elem->glow_color = glow_color;
//     elem->glow_thickness = glow_thickness;
//     elem->draw_glow = true;
// }

// //NOTE: Only works for static text or string literals where lifetime is not scoped.
// //If the src_text string gets deallocated then this is undefined behavior
// void element_settext(UiContext* uictx, ElementId id, const char* src_text) {
//     get_elementbyid(uictx, id)->text = src_text;
// }

// void element_removeglow(UiContext* uictx, ElementId id) {
//     get_elementbyid(uictx, id)->draw_glow = false;
// }

// void element_setvisibility(UiContext* uictx, ElementId id, bool is_visible) {
//     get_elementbyid(uictx, id)->visible = is_visible;
// }

// void element_setrectanglevisibility(UiContext* uictx, ElementId id, bool draw_rect) {
//     get_elementbyid(uictx, id)->draw_rect = draw_rect;
// }

// void element_setclickaction(UiContext* uictx, ElementId id, 
//             void* click_context, 
//             void(*onclick)(UiContext* ctx, ElementId clicked_element, void* click_context)) {
//     UiElement* e = get_elementbyid(uictx, id);
//     e->click_context = click_context;
//     e->click_action = onclick;
// }

// void element_clonesettingsfromid(UiContext* uictx, ElementId src_id, ElementId dst_id) {
//     UiElement* src_elem = get_elementbyid(uictx, src_id);
//     UiElement* dst_elem = get_elementbyid(uictx, dst_id);
//     *dst_elem = *src_elem;
//     dst_elem->id = dst_id;
// }

/*  --------------------------------------------------------------------------------------- /
                                    UiComboBox Functions
    --------------------------------------------------------------------------------------- */
// static UiComboBox* get_boxgroupbyid(UiContext* uictx, BoxGroupId id ){
//     return &((UiComboBox*)uictx->bg_arena.buffer)[id - 1];
// }


//Creates a combobox with elem_count number of elements inside of it
//Elements are allocated by this function
UiComboBox* combobox_create(UiContext* uictx, size_t elem_count) {
    UiComboBox* box = arena_alloc(&uictx->uibox_arena, sizeof(UiComboBox));
    *box = (UiComboBox){0};
    uictx->uibox_count++;

    box->first = arena_alloc(&uictx->elem_arena, sizeof(UiElement) * elem_count);
    box->count = elem_count;
    // box->selected_idx = 0;
    return box;
}

// bool boxgroup_addelement(UiContext* uictx, BoxGroupId bg_id, ElementId elem_id) {
//     UiComboBox* bg = get_boxgroupbyid(uictx, bg_id);
//     if (bg->count >= MAX_ELEMSPERBOXGROUP) {return false;}
//     bg->box_ids[bg->count] = elem_id;
//     bg->count++;
// }

// void boxgroup_setglow_selected(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness) {
//     UiComboBox* bg = get_boxgroupbyid(uictx, bg_id);
//     bg->selected_glow_color = glow_color;
//     bg->selected_glow_thickness = glow_thickness;
// }

// void boxgroup_setglow_hover(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness) {
//     UiComboBox* bg = get_boxgroupbyid(uictx, bg_id);
//     bg->hover_glow_color = glow_color;
//     bg->hover_glow_thickness = glow_thickness;
// }

// bool boxgroup_containselement(UiComboBox* bg, ElementId elem_id) {
//     for (int i = 0; i < bg->count; i++) {
//         if (bg->box_ids[i] == elem_id) {
//             return true;
//         }
//     }
//     return false;
// }

// void boxgroup_setselected(UiContext* uictx, UiComboBox* bg, ElementId elem_id) {
//     if (bg->selected) {
//         element_removeglow(uictx, bg->selected);
//     }
    
//     bg->selected = elem_id;
//     element_setglow(uictx, elem_id, bg->selected_glow_color, bg->selected_glow_thickness);
// }

// void boxgroup_sethover(UiContext* uictx, UiComboBox* bg, ElementId elem_id) {
//     if (elem_id == bg->selected) {
//         return;
//     }

//     if (bg->hovered) {
//         element_removeglow(uictx, bg->hovered);
//     }
    
//     bg->hovered = elem_id;
//     element_setglow(uictx, elem_id, bg->hover_glow_color, bg->hover_glow_thickness);
// }


//set default values for UI element (zeroing where appropriate)
//DOES NOT CONSTRUCT ELEMENT
//Relies on GetFontDefault from raylib
// void init_uielement(UiElement* element) {
//     element->draw_rect = true;
//     element->use_texture = false;
//     element->rect = (Rectangle){0};
//     element->inner_color = WHITE;
//     element->border_thickness = 0;
//     element->border_color = BLACK;
//     element->texture_rect = (Rectangle){0};
//     element->inner_texture = (Texture2D){0};


//     element->draw_glow = false;
//     element->glow_thickness = 0;
//     glow_color = (Color){0};
    
//     element->text = NULL;
//     element->text_color = BLACK;
//     element->text_font = GetFontDefault();
//     element->text_size = 12.0f;
//     element->text_spacing = 1.0f;
//     element->text_align = ALIGN_CENTER;
// }



// //Adds a number of elements to the GUI, returns pointer to first in the new range of elements.
// //If there's not enough room, elem_arena buffer is reallocated.
// UiElement* elemarena_addelems(ElementArena* elem_arena, size_t count) {
//     if (count + elem_arena->count > elem_arena->max) {
//         int newmax = (count + elem_arena->count) * 1.5;
//         UiElement* newbuf = malloc(sizeof(UiElement) * newmax);
//         for (int i = 0; i < elem_arena->count; i++) {
//             newbuf[i] = elem_arena->buffer[i];
//         }
//         free(elem_arena->buffer);
//         elem_arena->buffer = newbuf;
//         elem_arena->max = newmax;
//     }

//     UiElement* start = &elem_arena->buffer[elem_arena->count];
//     elem_arena->count += count;
//     return start;
// }


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
                                    Drawing Functions
    --------------------------------------------------------------------------------------- */
void update_uicontext(UiContext* uictx) {
    Vector2 mouse_pos = GetMousePosition();
    // uictx->mouse_clicked_elemID = 0;
    // uictx->mouse_over_elemID = 0;

    //Handle mouse events
    for (int i = 0; i < uictx->elem_count; i++) {
        UiElement* elem = (UiElement*)uictx->elem_arena.head->data;
        elem->focused = is_inelementbounds(elem, mouse_pos) ? true : false;
        elem->selected =  IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && elem->focused ? true: false;

        if (elem->selected && elem->click_action) {
            elem->click_action(uictx, elem);
        }
    }
}



static void draw_uielement(UiElement* element) {
    UiTheme* theme = element->theme;
    if (element->draw_glow) {
        Color glow_color = theme->glow_color_default;
        if (element->selected) {
            glow_color = theme->glow_color_selected;
        } else if (element->focused) {
            glow_color = theme->glow_color_focused;
        }
        DrawRectangleGradientV(element->rect.x,
                    element->rect.y - theme->glow_thickness,
                    element->rect.width,
                    theme->glow_thickness,
                    (Color){glow_color.r,glow_color.g,glow_color.b,0},
                    glow_color);
        DrawRectangleGradientV(element->rect.x,
                    element->rect.y + element->rect.height,
                    element->rect.width,
                    theme->glow_thickness,
                    glow_color,
                    (Color){glow_color.r,glow_color.g,glow_color.b,0});
        DrawRectangleGradientH(element->rect.x - theme->glow_thickness,
                    element->rect.y,
                    theme->glow_thickness,
                    element->rect.height,
                    (Color){glow_color.r,glow_color.g,glow_color.b,0},
                    glow_color);
        DrawRectangleGradientH(element->rect.x + element->rect.width,
                    element->rect.y,
                    theme->glow_thickness,
                    element->rect.height,
                    glow_color,
                    (Color){glow_color.r,glow_color.g,glow_color.b,0});
    }

    
    if (element->draw_rect) {
        Color border_color = theme->border_color_default;
        if (element->selected) {
            border_color = theme->border_color_selected;
        } else if (element->focused) {
            border_color = theme->border_color_focused;
        }
        if (theme->border_thickness) {
            Rectangle border_rect = (Rectangle){
                element->rect.x - theme->border_thickness,
                element->rect.y - theme->border_thickness,
                element->rect.width + theme->border_thickness * 2,
                element->rect.height + theme->border_thickness * 2
                };
            DrawRectangleRec(border_rect, theme->border_color);
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


void draw_uicontext(UiContext* uictx) {
    for (UiElement* elem = (UiElement*)uictx->elem_arena.buffer; 
        elem < (UiElement*)uictx->elem_arena.free; elem++) {
        draw_uielement(elem);
    }
}

// void draw_uiboxgroup(UiComboBox* group) {
//     //handle glow effect based on selected
//     //handle glow effect based on hover
//     for (int i = 0; i < group->count; i++) {
//         draw_uielement(&group->boxes[i]);
//     }
// }
