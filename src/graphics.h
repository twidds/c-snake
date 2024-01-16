#ifndef SNAKE_GRAPHICS_H
#define SNAKE_GRAPHICS_H

//TODO:: Make this platform independent (wrappers for windows/linux)

#include <gdiplus.h>

typedef struct Point2D {
    long x;
    long y;
} Point2D;

typedef struct Sprite {
    char* name;
    Point2D pixel_dimxy;
} Sprite;

//PlaceHolder:
typedef struct SpriteSheet {
    char* filepath;
    char* name;
    Point2D sprite_dimxy;
    Point2D pixel_dimxy;
    Sprite* sprites;
    GpImage* image;
} SpriteSheet;

typedef struct GraphicsData {
    GpImage* render_buffer;
    GpImage* spritesheet;
    // SpriteSheet* spritesheets;
} GraphicsData;

typedef enum RotateType {
    RotateNone,
    Rotate90,
    Rotate180,
    Rotate270
} RotateType;

ULONG_PTR setup_graphics(GraphicsData* graphics_data, HWND hwnd);
ARGB CreateARGB(BYTE a, BYTE r, BYTE g, BYTE b);
GpStatus DrawImageRectangle(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect);
GpStatus DrawRotatedImage(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect, RotateType rtype);

//PlaceHolders:
GpStatus DrawSprite(GpGraphics* graphics, SpriteSheet* spritesheet, Point2D* sprite_xy, GpRect* dst_rect, GpRect* src_rect);
GpStatus DrawRotatedSprite(GpGraphics* graphics, SpriteSheet* spritesheet, Point2D* sprite_xy, GpRect* dst_rect, GpRect* src_rect, RotateType rtype);


#endif /* SNAKE_GRAPHICS_H */

