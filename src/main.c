//References for future stuffs:
    //UTF-8 in WINAPI: https://stackoverflow.com/questions/8831143/windows-api-ansi-functions-and-utf-8
    //  https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    //  GetCommandLine -> GetCommandLineW
    //  MultiByteToWideChar
    //  Convert UTF-8 to UTF-16 before passing to WinAPI
    //  Should probably use W version of all WinAPI calls (A gets converted to W eventually anyway)


#include <stdio.h>
//#include <string.h>
//#include <malloc.h>
//#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <gdiplus.h>
//#pragma comment(lib, "gdiplus.lib")

#define WIDTH 100
#define HEIGHT 100


typedef enum object_type {
    INVALID,
    APPLE,
    SNAKE,
    SNAKE_HEAD,
    SNAKE_TAIL,
    SNAKE_BODY,
    TYPE_END
} object_type;

typedef enum direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction;

typedef struct node {
    object_type type;
    int x_pos;
    int y_pos;
    direction dir;
} node;

typedef struct snake {
    object_type type;
    struct node* nodes; //points at head
    int length;
    int max_length;
} snake;


typedef struct game_state {
    GpImage* test_image;
    wchar_t* front_buffer;
    wchar_t* back_buffer;
    snake game_snake;
    node apple;
} game_state;

void setconsole(int w, int h);

void clearconsole();

void swap_buffers(wchar_t** bufa, wchar_t** bufb) {
    wchar_t* tmp = *bufa;
    *bufa = *bufb;
    *bufb = tmp;
}

void render(wchar_t* buffer, void* object){
    enum object_type* t = object;
    switch(*t) {
        case APPLE:
            break;
        case SNAKE:
            break;
        case SNAKE_HEAD:
            break;
        case SNAKE_BODY:
            break;
        case SNAKE_TAIL:
            break;
    }
}


typedef enum RotateType {
    RotateNone,
    Rotate90,
    Rotate180,
    Rotate270
} RotateType;

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
    GpStatus status = Ok;

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

    status = DrawImageRectangle(graphics, image, dst_rect, src_rect);
    
    if (status != Ok) {return status;}
    
    return GdipResetWorldTransform(graphics);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //on window creation, set game state pointer
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(pCreate->lpCreateParams));
        return TRUE;
    }

    //handle window messages    
    game_state* state = (game_state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
            {
                // const wchar_t* message = L"WWWWWWWWWW";
                // const wchar_t* msg2 = L"\U000021D7 wwwwwwwwww";
                PAINTSTRUCT ps;
                RECT rc;
                GetClientRect(hwnd, &rc);
                HDC hdc = BeginPaint(hwnd, &ps);

                GpGraphics* graphics;
                GdipCreateFromHDC(hdc, &graphics);

                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
                //GdipDrawImage(graphics, state->test_image, 0, 0);
                //GdipDrawImageRect(graphics, state->test_image, 
                //    0, 0, //x, y
                //    50,50 //width, height
                //    );
                GpRect img_rect = {};
                GdipGetImageWidth(state->test_image, &(img_rect.Width));
                GdipGetImageHeight(state->test_image, &(img_rect.Height));

                //GpStatus WINGDIPAPI GdipDrawImagePoints(GpGraphics *graphics, GpImage *image, GDIPCONST GpPointF *dstpoints, INT count)
                GpRect dst_rect = {};
                dst_rect.Width = 200;
                dst_rect.Height = 200;

                DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);
                dst_rect.X = dst_rect.Width;
                DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);
                dst_rect.X = 0;
                dst_rect.Y = dst_rect.Height;
                DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);
                dst_rect.X = dst_rect.Width;
                DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);

                dst_rect.X = 2*dst_rect.Width; dst_rect.Y = 2*dst_rect.Height;
                DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);

                // DrawText(hdc, message, -1, &rc, DT_TOP | DT_LEFT);
                // rc.top += 50;
                // DrawText(hdc, msg2, -1, &rc, DT_TOP | DT_LEFT);
                
                EndPaint(hwnd, &ps);
            }
            return 0;
        
        case WM_KEYDOWN:
            //Check if arrow keys and handle changing snake direction
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void setup_game(game_state* state) {
    state->front_buffer = malloc(WIDTH * HEIGHT * sizeof(wchar_t));
    state->back_buffer = malloc(WIDTH * HEIGHT * sizeof(wchar_t));
    state->game_snake.length = 2; //just the head and tail
    state->game_snake.max_length = 10; //start with space for 10 segments
    state->game_snake.nodes = malloc(sizeof(node) * state->game_snake.max_length);
    state->game_snake.nodes[0].dir = LEFT;
    state->game_snake.nodes[1].dir = LEFT;
    state->game_snake.nodes[0].type = SNAKE_HEAD;
    state->game_snake.nodes[1].type = SNAKE_TAIL;
    //should center snake on screen
    state->game_snake.nodes[1].x_pos += 1; //tail should be offset from head
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    //TODO:: Set window size based on command line
    //  __argv, __argc

    //GDI+ init
    GdiplusStartupInput gdip_input;
    gdip_input.GdiplusVersion = 1; // Undocumented? Required for it to work correctly.
    GdiplusStartupOutput gdip_output;
    ULONG_PTR gdip_token;
    GpStatus gp_status;

    gp_status = GdiplusStartup(&gdip_token, &gdip_input, &gdip_output);
    if (gp_status != Ok) {
        return gp_status;
    }
    
    //should move to import_assets function
    GpImage* test_image;
    gp_status = GdipLoadImageFromFile(L"../assets/smiley.png", &test_image);
    if (gp_status != Ok) {
        return gp_status;
    }
    
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Snake Main Window";
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    
    game_state state = {};
    state.test_image = test_image;
    setup_game(&state);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Snake",    // Window text
        //WS_OVERLAPPEDWINDOW,            // Window style
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // dwStyle, X, Y, nWidth, nHeight
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        &state        // Additional application data
        );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    //Game loop start

    //Process message queue (care about keyboard input)
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {  // Change to PeekMessage to make non-blocking
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //Render to some target buffer? Probably a global...
    
    //Queue redraw
    //  Call InvalidateRgn to queue entire client area for redraw
    //  Create WM_PAINT event and dispatch it
    

    //Set timer to remaining time for vsync (Use SetTimer winapi to trigger an event?)

    //Game loop end

    //cleanup (not needed, just good practice)
    free(state.game_snake.nodes);
    free(state.front_buffer);
    free(state.back_buffer);
    GdiplusShutdown(gdip_token);

    return 0;       
}
