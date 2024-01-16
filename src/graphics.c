#include <windows.h>
#include "graphics.h"


ARGB CreateARGB(BYTE a, BYTE r, BYTE g, BYTE b) {
    return a << 24 | r << 16 | g << 8 | b;
}

GpStatus DrawImageRectangle(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect) {
    return GdipDrawImageRectRectI(
            graphics,                //GpGraphics *graphics,
            image,       //GpImage *image,
            dst_rect->X, dst_rect->Y, dst_rect->Width, dst_rect->Height,    //INT dstx, INT dsty, INT dstwidth, INT dstheight,
            src_rect->X, src_rect->Y, src_rect->Width, src_rect->Height,  //INT srcx, INT srcy, INT srcwidth, INT srcheight,
            UnitPixel,               //GpUnit srcUnit,
            NULL,                    //GDIPCONST GpImageAttributes* imageAttributes,
            NULL,                    //DrawImageAbort callback,
            NULL                     //VOID * callbackData
            );
}

GpStatus DrawImageRectangleRotated(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect, RotateType rtype) {
    GpStatus gp_status = Ok;

    switch(rtype) {
        case RotateNone:
            GdipTranslateWorldTransform(graphics, dst_rect->X, dst_rect->Y, MatrixOrderAppend);
            break;
        case Rotate90:
            GdipRotateWorldTransform(graphics, 90.0f, MatrixOrderAppend);
            GdipTranslateWorldTransform(graphics, 
                (dst_rect->X + dst_rect->Y + dst_rect->Height)*0.999,
                (dst_rect->Y - dst_rect->X)*0.999,
                MatrixOrderAppend);
            break;
        case Rotate180:
            GdipRotateWorldTransform(graphics, 180.0f, MatrixOrderAppend);
            GdipTranslateWorldTransform(graphics, 
                (2*dst_rect->X + dst_rect->Width)*0.999, 
                (2*dst_rect->Y + dst_rect->Height)*0.999, 
                MatrixOrderAppend);
            break;
        case Rotate270:
            GdipRotateWorldTransform(graphics, 270.0f, MatrixOrderAppend);
            GdipTranslateWorldTransform(graphics,
                (dst_rect->X - dst_rect->Y)*0.999,
                (dst_rect->Width + dst_rect->X + dst_rect->Y)*0.999,
                MatrixOrderAppend);
            break;
        default:
            return InvalidParameter;
            break;
    }

    gp_status = DrawImageRectangle(graphics, image, dst_rect, src_rect);
    if (gp_status != Ok) {return gp_status;}
    
    gp_status = GdipResetWorldTransform(graphics);
    return gp_status;
}

//TODO:: Some of this should go into game.c. For example, loading the actual spritesheet
GrStatus setup_graphics(GraphicsData* graphics_data, HWND hwnd) {
    //GDI+ init
    GdiplusStartupInput gdip_input;
    gdip_input.GdiplusVersion = 1; // Required for proper gdip startup.
    GdiplusStartupOutput gdip_output; //Only use if we implement our own UI thread
    ULONG_PTR gdip_token;
    GpStatus gp_status;

    gp_status = GdiplusStartup(&graphics_data->gdip_token, &gdip_input, &gdip_output);
    if (gp_status != Ok) {
        return GRAPHICS_FAIL;
    }
    
    SpriteSheet* spritesheet = malloc(sizeof(SpriteSheet));
    gp_status = ImportSpriteSheet(spritesheet, L"../assets/spritesheet.bmp", 4, 5);
    if (gp_status != Ok) {
        free(spritesheet);
        GdiplusShutdown(gdip_token);
        return gp_status;
    }
    graphics_data->spritesheet = spritesheet;

    // gp_status = GdipLoadImageFromFile(L"../assets/spritesheet.bmp", &(graphics_data->spritesheet));
    // if (gp_status != Ok) {
    //     GdiplusShutdown(gdip_token);
    //     return GRAPHICS_FAIL;
    // }

    // gp_status = render_game(state, hwnd);
    // if (gp_status != Ok) {
    //     GdiplusShutdown(gdip_token);
    //     return NULL;
    // }

    return GRAPHICS_OK;
}

GrStatus cleanup_graphics(GraphicsData* graphics_data) {
    GpStatus status;
    if (graphics_data->render_buffer) {
        if (Ok != GdipDisposeImage(graphics_data->render_buffer)) {
            return GRAPHICS_FAIL;
        }
    }

    if (graphics_data->spritesheet) {
        if (Ok != GdipDisposeImage(graphics_data->spritesheet)) {
            return GRAPHICS_FAIL;
        }
    }

    GdiplusShutdown(graphics_data->gdip_token);
    return GRAPHICS_OK;
}

//NOTE: Do not pass in existing spreadsheet without freeing sprites first. This will leak.
GpStatus ImportSpriteSheet(SpriteSheet* spritesheet, const wchar_t* filepath, int n_cols, int n_rows) {
    GpStatus gp_status;
    gp_status = GdipLoadImageFromFile(filepath, &(spritesheet->image));
    if (gp_status != Ok) {
        return gp_status;
    }


    UINT width;
    UINT height;
    GdipGetImageWidth(spritesheet->image, &width);
    GdipGetImageHeight(spritesheet->image, &height);
    spritesheet->imagesize.x = width;
    spritesheet->imagesize.y = height;

    //check image size divisible by sprite size
    int rem_x = width % n_cols;
    int rem_y = height % n_rows;
    if (rem_x || rem_y) { 
        GdipDisposeImage(spritesheet->image);
        return GenericError;
    }

    spritesheet->spritesheet_size.x = n_cols;
    spritesheet->spritesheet_size.y = n_rows;
    spritesheet->sprite_size.x = width / n_cols;
    spritesheet->sprite_size.y = height / n_rows;
    spritesheet->sprite_count = n_cols * n_rows;

    //init sprites
    spritesheet->sprites = malloc(spritesheet->sprite_count * sizeof(Sprite));
    Sprite* sprite;
    for (int col = 0; col < n_cols; col++) {
        for (int row = 0; row < n_rows; row++) {
            sprite = &spritesheet->sprites[col + row * n_cols];
            sprite->image_rect.Height = spritesheet->sprite_size.y;
            sprite->image_rect.Width = spritesheet->sprite_size.x;
            sprite->image_rect.X = col * spritesheet->sprite_size.x;
            sprite->image_rect.Y = row * spritesheet->sprite_size.y;
        }
    }
    
    return gp_status;
}

GpStatus FreeSpriteSheet(SpriteSheet* spritesheet) {
    GpStatus status;
    
    status = GdipDisposeImage(spritesheet->image);
    if (status != Ok) {
        return status;
    }
    spritesheet->image = NULL;

    free(spritesheet->sprites);
    spritesheet->sprites = NULL;
    spritesheet->sprite_count = 0;

    return Ok;
}

//TODO:: Better to call into DrawSpriteRotated_Index? Could avoid a little duplication
GpStatus DrawSprite_Index(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, int sprite_index) {
    if (sprite_index >= spritesheet->sprite_count) {
        return GenericError;
    }

    GpRect* image_rect = &spritesheet->sprites[sprite_index].image_rect;
    return DrawImageRectangle(graphics, spritesheet->image, dst_rect, image_rect);
}

GpStatus DrawSprite_XY(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, Point2D sprite_xy) {
    int index = sprite_xy.y * spritesheet->spritesheet_size.x + sprite_xy.x;
    return DrawSprite_Index(graphics, dst_rect, spritesheet, index);
}

GpStatus DrawSpriteRotated_Index(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, int sprite_index, RotateType rtype) {
    if (sprite_index >= spritesheet->sprite_count) {
        return GenericError;
    }

    GpRect* image_rect = &spritesheet->sprites[sprite_index].image_rect;
    return DrawImageRectangleRotated(graphics, spritesheet->image, dst_rect, image_rect, rtype);
}

GpStatus DrawSpriteRotated_XY(GpGraphics* graphics, GpRect* dst_rect, SpriteSheet* spritesheet, Point2D sprite_xy, RotateType rtype) {
    int index = sprite_xy.y * spritesheet->spritesheet_size.x + sprite_xy.x;
    return DrawSpriteRotated_Index(graphics, dst_rect, spritesheet, index,  rtype);
}

