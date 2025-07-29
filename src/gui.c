#include "gui.h"
#include "raylib.h"
// #include <stdbool.h> //bool
#include <stdlib.h> //malloc, NULL

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




void elemarena_alloc(ElementArena* arena, int size) {
    if (arena->buffer) {free(arena->buffer);}
    arena->buffer = malloc(sizeof(UiElement) * size);
    arena->count = 0;
    arena->max = size;
}

void elemarena_dealloc(ElementArena* arena) {
    if (arena->buffer) {
        free(arena->buffer);
    }
    arena->count = 0;
    arena->max = 0;
}

//Adds a number of elements to the GUI, returns pointer to first in the new range of elements.
//If there's not enough room, arena buffer is reallocated.
UiElement* elemarena_addelems(ElementArena* arena, int count) {
    if (count + arena->count > arena->max) {
        int newmax = (count + arena->count) * 1.5;
        UiElement* newbuf = malloc(sizeof(UiElement) * newmax);
        for (int i = 0; i < arena->count; i++) {
            newbuf[i] = arena->buffer[i];
        }
        free(arena->buffer);
        arena->buffer = newbuf;
        arena->max = newmax;
    }

    UiElement* start = &arena->buffer[arena->count];
    arena->count += count;
    return start;
}


bool is_inelementbounds(UiElement* elem, Vector2 pos) {
    return  elem->rect.x < pos.x &&
            elem->rect.y < pos.y &&
            elem->rect.x + elem->rect.width > pos.x &&
            elem->rect.y + elem->rect.height > pos.y;
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