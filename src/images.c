
#include "images.h"
#include "images\backgrounds_spritesheet.h"
#include "images\food_spritesheet.h"
#include "images\snake_spritesheet.h"

static Texture2D textures[TEXTURE_COUNT] = {0};

void init_snaketextures() {
    Image img = {0};
    
    //Snake image
    img.format = SNAKE_SPRITESHEET_FORMAT;
    img.height = SNAKE_SPRITESHEET_HEIGHT;
    img.width = SNAKE_SPRITESHEET_WIDTH;
    img.data = SNAKE_SPRITESHEET_DATA;
    img.mipmaps = 1;
    textures[TEXTURE_SNAKE] = LoadTextureFromImage(img);

    //Food image
    img.format = FOOD_SPRITESHEET_FORMAT;
    img.height = FOOD_SPRITESHEET_HEIGHT;
    img.width = FOOD_SPRITESHEET_WIDTH;
    img.data = FOOD_SPRITESHEET_DATA;
    img.mipmaps = 1;
    textures[TEXTURE_FOOD] = LoadTextureFromImage(img);
    
    //Background image
    img.format = BACKGROUNDS_SPRITESHEET_FORMAT;
    img.height = BACKGROUNDS_SPRITESHEET_HEIGHT;
    img.width = BACKGROUNDS_SPRITESHEET_WIDTH;
    img.data = BACKGROUNDS_SPRITESHEET_DATA;
    img.mipmaps = 1;
    textures[TEXTURE_BACKGROUNDS] = LoadTextureFromImage(img);
}

Texture2D get_snaketexture(TextureIdx texture_idx) {
    return textures[texture_idx];
}