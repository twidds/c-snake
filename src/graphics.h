#ifndef SNAKE_GRAPHICS_H
#define SNAKE_GRAPHICS_H

//TODO:: Make this platform independent (wrappers for windows/linux)

#include <gdiplus.h>

typedef struct Point2D {
    long x;
    long y;
} Point2D;

typedef struct Sprite {
    GpRect image_rect; //Rectangle of pixels in image for this Sprite
} Sprite;

//PlaceHolder:
typedef struct SpriteSheet {
    Point2D spritesheet_size; //rows/column count of sprites
    Point2D sprite_size; //pixel x/y
    Sprite* sprites;
    int sprite_count;
    Point2D imagesize; //pixels
    GpImage* image;
} SpriteSheet;

typedef struct GraphicsData {
    ULONG_PTR gdip_token;
    GpImage* render_buffer;
    // GpImage* spritesheet;
    SpriteSheet* spritesheet;
} GraphicsData;

typedef enum RotateType {
    RotateNone,
    Rotate90,
    Rotate180,
    Rotate270
} RotateType;

typedef enum GrStatus {
    GRAPHICS_OK,
    GRAPHICS_FAIL,
} GrStatus;



GrStatus setup_graphics(GraphicsData* graphics_data, HWND hwnd);
GrStatus cleanup_graphics(GraphicsData* graphics_data);
ARGB CreateARGB(BYTE a, BYTE r, BYTE g, BYTE b);
GpStatus DrawImageRectangle(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect);
GpStatus DrawImageRectangleRotated(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect, RotateType rtype);

//PlaceHolders:
GpStatus ImportSpriteSheet(SpriteSheet* spritesheet, const wchar_t* filepath, int n_cols, int n_rows);
GpStatus FreeSpriteSheet(SpriteSheet* spritesheet);
GpStatus DrawSprite_XY(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, Point2D sprite_xy);
GpStatus DrawSprite_Index(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, int sprite_index);
GpStatus DrawSpriteRotated_XY(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, Point2D sprite_xy, RotateType rtype);
GpStatus DrawSpriteRotated_Index(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, int sprite_index, RotateType rtype);


#endif /* SNAKE_GRAPHICS_H */

