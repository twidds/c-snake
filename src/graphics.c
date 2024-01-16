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

GpStatus DrawRotatedImage(GpGraphics* graphics, GpImage* image, GpRect* dst_rect, GpRect* src_rect, RotateType rtype) {
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
    
    gp_status = GdipLoadImageFromFile(L"../assets/spritesheet.bmp", &(graphics_data->spritesheet));
    if (gp_status != Ok) {
        GdiplusShutdown(gdip_token);
        return GRAPHICS_FAIL;
    }

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