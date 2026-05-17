#ifndef SNAKE_IMAGES_H
#define SNAKE_IMAGES_H

#include "raylib.h"

typedef enum {
    TEXTURE_SNAKE,
    TEXTURE_FOOD,
    TEXTURE_BACKGROUNDS,
    TEXTURE_COUNT
} TextureIdx;

void init_snaketextures();
Texture2D get_snaketexture(TextureIdx texture_idx);

#endif