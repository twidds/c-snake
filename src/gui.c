#include "gui.h"
#include <stdlib.h> //malloc, NULL

#define ELEM_ARENA_STARTSIZE 10 * sizeof(UiElement)
#define BOXGROUP_ARENA_STARTSIZE 10 * sizeof(UiBoxGroup)
#define TEXT_ARENA_STARTSIZE 1024 * sizeof(char)
#define BOXGROUP_STARTSIZE 10
#define ARENA_DEFAULTSIZE 2048 //bytes


/*  --------------------------------------------------------------------------------------- /
                                Arena Memory Functions
    --------------------------------------------------------------------------------------- */
static void arena_init_withsize(Arena* arena, size_t size) {
    arena->buffer = malloc(sizeof(char) * size);
    arena->free = arena->buffer;
    arena->end = arena->buffer + size;
    arena->next_id = 1;
}

static void arena_init(Arena* arena) {
    arena_init_withsize(arena, ARENA_DEFAULTSIZE);
}

static void* arena_alloc(Arena* arena, size_t size, size_t count){
    size_t bump_bytes = size * count;
    if (arena->free + bump_bytes > arena->end) {
        return NULL;
    }
    void* addr = arena->free;
    arena->free += bump_bytes;
    return addr;
}

static void arena_grow(Arena* arena, unsigned int factor) {
    size_t count = arena->free - arena->buffer;

    char* newbuf = malloc(count * factor);
    for (int i = 0; i > count; i){
        newbuf[i] = arena->buffer[i];
    }

    free(arena->buffer);
    arena->buffer = newbuf;
    arena->end = newbuf + (count*factor);
    arena->free = arena->buffer + count;
}

static void arena_reset(Arena* arena) {
    arena->free = arena->buffer;
}

static void arena_destroy(Arena* arena) {
    free(arena->buffer);
    arena->buffer = NULL;
}


/*  --------------------------------------------------------------------------------------- /
                                    UiGroup Functions
    --------------------------------------------------------------------------------------- */
void setup_uicontext(UiContext* uictx) {
    *uictx = (UiContext){0};
    arena_init_withsize(&uictx->elem_arena, ELEM_ARENA_STARTSIZE);
    arena_init_withsize(&uictx->bg_arena, BOXGROUP_ARENA_STARTSIZE);
}

void destroy_uicontext(UiContext* uictx) {
    arena_destroy(&uictx->elem_arena);
    arena_destroy(&uictx->bg_arena);
}


/*  --------------------------------------------------------------------------------------- /
                                    UiElement Functions
    --------------------------------------------------------------------------------------- */
static UiElement* get_elementbyid(UiContext* uictx, ElementId id){
    return &((UiElement*)uictx->elem_arena.buffer)[id - 1];
}


ElementId element_create(UiContext* uictx) {
    UiElement* elem = arena_alloc(&uictx->elem_arena, sizeof(UiElement), 1);
    while (!elem){
        arena_grow(&uictx->elem_arena, 1.5);
        elem = arena_alloc(&uictx->elem_arena, sizeof(UiElement), 1);
    }
    *elem = (UiElement){0};
    elem->id = uictx->elem_arena.next_id;
    uictx->elem_arena.next_id++;
    elem->visible = true;
    return elem->id;
}


ElementId element_createbox(UiContext* uictx, Rectangle draw_rectangle) {
    ElementId id = element_create(uictx);
    element_setdrawrectangle(uictx, id, draw_rectangle);
    return id;
}

void element_settexture(UiContext* uictx, ElementId id, Texture2D texture, Rectangle texture_rectangle) {
    UiElement* elem = get_elementbyid(uictx, id);
    elem->inner_texture = texture;
    elem->texture_rect = texture_rectangle;
}

void element_enabletexture(UiContext* uictx, ElementId id) {
    get_elementbyid(uictx, id)->use_texture = true;
}

void element_disabletexture(UiContext* uictx, ElementId id) {
    get_elementbyid(uictx, id)->use_texture = false;
}

void element_setcolor(UiContext* uictx, ElementId id, Color color){
    get_elementbyid(uictx, id)->inner_color = color;
}

void element_setborder(UiContext* uictx, ElementId id, Color border_color, int border_thickness) {
    UiElement* elem = get_elementbyid(uictx, id);
    elem->border_color = border_color;
    elem->border_thickness = border_thickness;
}

void element_setposition(UiContext* uictx, ElementId id, iVec2D position) {
    UiElement* elem = get_elementbyid(uictx, id);
    elem->rect.x = position.x;
    elem->rect.y = position.y;
}

void element_setwidthheight(UiContext* uictx, ElementId id, int width, int height) {
    UiElement* elem = get_elementbyid(uictx, id);
    elem->rect.width = width;
    elem->rect.height = height;
}

void element_setdrawrectangle(UiContext* uictx, ElementId id, Rectangle rectangle) {
    get_elementbyid(uictx, id)->rect = rectangle;
}

void element_setglow(UiContext* uictx, ElementId id, Color glow_color, int glow_thickness) {
    UiElement* elem = get_elementbyid(uictx, id);
    elem->glow_color = glow_color;
    elem->glow_thickness = glow_thickness;
    elem->draw_glow = true;
}

//NOTE: Only works for static text or string literals where lifetime is not scoped.
//If the src_text string gets deallocated then this is undefined behavior
void element_settext(UiContext* uictx, ElementId id, const char* src_text) {
    get_elementbyid(uictx, id)->text = src_text;
}

void element_removeglow(UiContext* uictx, ElementId id) {
    get_elementbyid(uictx, id)->draw_glow = false;
}

void element_setvisibility(UiContext* uictx, ElementId id, bool is_visible) {
    get_elementbyid(uictx, id)->visible = is_visible;
}

void element_setrectanglevisibility(UiContext* uictx, ElementId id, bool draw_rect) {
    get_elementbyid(uictx, id)->draw_rect = draw_rect;
}

void element_setclickaction(UiContext* uictx, ElementId id, 
            void* click_context, 
            void(*onclick)(UiContext* ctx, ElementId clicked_element, void* click_context)) {
    UiElement* e = get_elementbyid(uictx, id);
    e->click_context = click_context;
    e->click_action = onclick;
}

void element_clonesettingsfromid(UiContext* uictx, ElementId src_id, ElementId dst_id) {
    UiElement* src_elem = get_elementbyid(uictx, src_id);
    UiElement* dst_elem = get_elementbyid(uictx, dst_id);
    *dst_elem = *src_elem;
    dst_elem->id = dst_id;
}

/*  --------------------------------------------------------------------------------------- /
                                    UiBoxGroup Functions
    --------------------------------------------------------------------------------------- */
static UiBoxGroup* get_boxgroupbyid(UiContext* uictx, BoxGroupId id ){
    return &((UiBoxGroup*)uictx->bg_arena.buffer)[id - 1];
}

BoxGroupId boxgroup_create(UiContext* uictx) {
    UiBoxGroup* group = arena_alloc(&uictx->bg_arena, sizeof(UiBoxGroup), 1);
    while (!group){
        arena_grow(&uictx->bg_arena, 1.5);
        group = arena_alloc(&uictx->bg_arena, sizeof(UiBoxGroup), 1);
    }
    *group = (UiBoxGroup){0};
    group->id = uictx->bg_arena.next_id;
    uictx->bg_arena.next_id++;
    return group->id;
}

bool boxgroup_addelement(UiContext* uictx, BoxGroupId bg_id, ElementId elem_id) {
    UiBoxGroup* bg = get_boxgroupbyid(uictx, bg_id);
    if (bg->count >= MAX_ELEMSPERBOXGROUP) {return false;}
    bg->box_ids[bg->count] = elem_id;
    bg->count++;
}

void boxgroup_setglow_selected(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness) {
    UiBoxGroup* bg = get_boxgroupbyid(uictx, bg_id);
    bg->selected_glow_color = glow_color;
    bg->selected_glow_thickness = glow_thickness;
}

void boxgroup_setglow_hover(UiContext* uictx, BoxGroupId bg_id, Color glow_color, int glow_thickness) {
    UiBoxGroup* bg = get_boxgroupbyid(uictx, bg_id);
    bg->hover_glow_color = glow_color;
    bg->hover_glow_thickness = glow_thickness;
}

bool boxgroup_containselement(UiBoxGroup* bg, ElementId elem_id) {
    for (int i = 0; i < bg->count; i++) {
        if (bg->box_ids[i] == elem_id) {
            return true;
        }
    }
    return false;
}

void boxgroup_setselected(UiContext* uictx, UiBoxGroup* bg, ElementId elem_id) {
    if (bg->selected) {
        element_removeglow(uictx, bg->selected);
    }
    
    bg->selected = elem_id;
    element_setglow(uictx, elem_id, bg->selected_glow_color, bg->selected_glow_thickness);
}

void boxgroup_sethover(UiContext* uictx, UiBoxGroup* bg, ElementId elem_id) {
    if (elem_id == bg->selected) {
        return;
    }

    if (bg->hovered) {
        element_removeglow(uictx, bg->hovered);
    }
    
    bg->hovered = elem_id;
    element_setglow(uictx, elem_id, bg->hover_glow_color, bg->hover_glow_thickness);
}


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
//     element->glow_color = (Color){0};
    
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
    uictx->mouse_clicked_elemID = 0;
    uictx->mouse_over_elemID = 0;

    //Handle mouse events
    for (UiElement* elem = (UiElement*)uictx->elem_arena.buffer; elem < (UiElement*)uictx->elem_arena.free; elem++) {
        if (is_inelementbounds(elem, mouse_pos)) {
            uictx->mouse_over_elemID = elem->id;
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && uictx->mouse_over_elemID) {
        uictx->mouse_down_elemID = uictx->mouse_over_elemID;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (uictx->mouse_over_elemID == uictx->mouse_down_elemID) {
            uictx->mouse_clicked_elemID = uictx->mouse_over_elemID;
        }
        uictx->mouse_down_elemID= 0;
    }

    //Update box group clicks/hovers
    if (uictx->mouse_clicked_elemID) {
        for (UiBoxGroup* bg = (UiBoxGroup*)uictx->bg_arena.buffer; bg < (UiBoxGroup*)uictx->bg_arena.free; bg++){
            if (boxgroup_containselement(bg, uictx->mouse_clicked_elemID)) {
                boxgroup_setselected(uictx, bg, uictx->mouse_clicked_elemID);
            }
        }
    }
    if (uictx->mouse_over_elemID) {
        for (UiBoxGroup* bg = (UiBoxGroup*)uictx->bg_arena.buffer; bg < (UiBoxGroup*)uictx->bg_arena.free; bg++){
            if (boxgroup_containselement(bg, uictx->mouse_over_elemID)) {
                boxgroup_sethover(uictx, bg, uictx->mouse_over_elemID);
            }
        }
    }

    //Call user click action
    if (uictx->mouse_clicked_elemID) {
        UiElement* elem = get_elementbyid(uictx, uictx->mouse_clicked_elemID);
        if (elem->click_action) {
            elem->click_action(uictx, elem->id, elem->click_context);
        }
    }
}



static void draw_uielement(UiElement* element) {
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


void draw_uicontext(UiContext* uictx) {
    for (UiElement* elem = (UiElement*)uictx->elem_arena.buffer; 
        elem < (UiElement*)uictx->elem_arena.free; elem++) {
        draw_uielement(elem);
    }
}

// void draw_uiboxgroup(UiBoxGroup* group) {
//     //handle glow effect based on selected
//     //handle glow effect based on hover
//     for (int i = 0; i < group->count; i++) {
//         draw_uielement(&group->boxes[i]);
//     }
// }
