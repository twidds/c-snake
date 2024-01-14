//References for future stuffs:
    //UTF-8 in WINAPI: https://stackoverflow.com/questions/8831143/windows-api-ansi-functions-and-utf-8
    //  https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    //  GetCommandLine -> GetCommandLineW
    //  MultiByteToWideChar
    //  Convert UTF-8 to UTF-16 before passing to WinAPI
    //  Should probably use W version of all WinAPI calls (A gets converted to W eventually anyway)


#include <stdio.h>
#include <stdbool.h>
//#include <string.h>
//#include <malloc.h>
//#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
    #define UNICODE
#endif
#include <windows.h>
#include <gdiplus.h>

//TODO:: Make size dynamic, use these for min sizes.
#define MIN_WIDTH 10
#define MIN_HEIGHT 10

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define WIDTH 800
#define HEIGHT 800

typedef enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct Node {
    // SnakeObjectType type;
    // Direction facing;
    int x_pos;
    int y_pos;
} Node;

typedef struct Snake {
    // SnakeObjectType type;
    Direction buffered_dir;
    Direction move_dir;
    Node* nodes; //points at tail
    int length;
    int max_length;
} Snake;

typedef struct CmdConfig {
    int window_x;
    int window_y;
    int board_x;
    int board_y;
} CmdConfig;

typedef struct GameState {
    bool show_stats;
    int n_renders;
    int n_draws;
    bool paused;
    bool pause_pending;
    bool running;
    unsigned int score;
    POINT window_size;
    POINT board_size;
    Snake game_snake;
    Node apple;
    
    //Graphics
    GpImage* spritesheet;
    GpImage* drawbuffer;
} GameState;

void setconsole(int w, int h);

void clearconsole();

//sets bufa to bufb, then bufb to old bufa
void swap_buffers(void** bufa, void** bufb) {
    void* tmp = *bufa;
    *bufa = *bufb;
    *bufb = tmp;
}

typedef enum RotateType {
    RotateNone,
    Rotate90,
    Rotate180,
    Rotate270
} RotateType;


ARGB make_ARGB(BYTE a, BYTE r, BYTE g, BYTE b) {
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


LRESULT PaintWindow(HWND hwnd, GameState* state) {
    // const wchar_t* message = L"WWWWWWWWWW";
    // const wchar_t* msg2 = L"\U000021D7 wwwwwwwwww";
    // printf("paint call\r\n");
    if (!state->spritesheet) {
        return 0;
    }
    GpStatus status;

    //Put draw from cached bitmap here:
    state->n_draws++;
    PAINTSTRUCT ps;
    RECT rc;
    InvalidateRgn(hwnd, NULL, FALSE);
    GetClientRect(hwnd, &rc);
    HDC hdc = BeginPaint(hwnd, &ps);

    GpGraphics* graphics;
    status = GdipCreateFromHDC(hdc, &graphics);
    status = GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);

    // status = GdipDrawCachedBitmap(graphics, state->drawbuffer, 0, 0);
    status = GdipDrawImage(graphics, state->drawbuffer, 0, 0);

    //draw stats
    if (state->show_stats) {
        char statstring[30];
        wchar_t statstringW[30];
        
        sprintf(statstring, "draws: %i", state->n_draws);
        MultiByteToWideChar(CP_UTF8, 0, statstring, -1, statstringW, 30);
        rc.top += DrawText(hdc, statstringW, -1, &rc, DT_TOP | DT_LEFT);

        sprintf(statstring, "renders: %i", state->n_renders);
        MultiByteToWideChar(CP_UTF8, 0, statstring, -1, statstringW, 30);
        DrawText(hdc, statstringW, -1, &rc, DT_TOP | DT_LEFT);
    }
    
    EndPaint(hwnd, &ps);
    GdipDeleteGraphics(graphics);
    return 0;
}

LRESULT HandleKeys(HWND hwnd, GameState* state, WPARAM wParam, LPARAM lParam) {
    
    switch(wParam) { //special keys
        case VK_ESCAPE: //exit
            state->running = false;
            break;
        case 'P': //pause
            state->paused = !(state->paused);
            state->pause_pending = state->paused;
            break;
        case 'S': //stats
            state->show_stats = !(state->show_stats);
            break;
    }

    if (state->paused) { //skip rest of keys
        return 0;
    }
    
    switch(wParam) { //gameplay keys
        case VK_LEFT:
            if (state->game_snake.move_dir != RIGHT) {
                state->game_snake.buffered_dir = LEFT;
            }
            break;
        case VK_UP:
            if (state->game_snake.move_dir != DOWN) {
                state->game_snake.buffered_dir = UP;
            }
            break;
        case VK_RIGHT:
            if (state->game_snake.move_dir != LEFT) {
                state->game_snake.buffered_dir = RIGHT;
            }
            break;
        case VK_DOWN:
            if (state->game_snake.move_dir != UP) {
                state->game_snake.buffered_dir = DOWN;
            }
            break;
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //on window creation, set game state pointer
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(pCreate->lpCreateParams));
        return TRUE;
    }

    //handle window messages    
    GameState* state = (GameState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_ERASEBKGND:
            return TRUE;
        case WM_DESTROY:
            // PostQuitMessage(0);
            state->running = false;
            return 0;
        case WM_PAINT:
            return PaintWindow(hwnd, state);
        case WM_KEYDOWN:
            //Check if arrow keys and handle changing snake direction
            return HandleKeys(hwnd, state, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void realloc_snake_nodes(Snake* snake) {
    int newmax = 2 * snake->max_length;
    Node* new_nodes = malloc(sizeof(Node) * newmax);
    memcpy(new_nodes, snake->nodes, sizeof(Node) * snake->length);
    free(snake->nodes);
    snake->nodes = new_nodes;
}

int setup_game(GameState* state, CmdConfig* config) {
    //graphics to fill in later
    state->drawbuffer = NULL;
    state->spritesheet = NULL;

    //from cmd line
    state->window_size.x = config->window_x;
    state->window_size.y = config->window_y;
    state->board_size.x = config->board_x;
    state->board_size.y = config->board_y;
    
    //game initial state
    state->running = true;
    state->paused = false;
    state->score = 0;
    state->n_draws = 0;
    state->n_renders = 0;
    state->show_stats = true;
    
    //setup snake
    int start_x = state->board_size.x/2;
    int start_y = state->board_size.y/2;
    int max_nodes = 10;

    Snake* snake = &(state->game_snake);
    snake->max_length = max_nodes;
    snake->nodes = malloc(sizeof(Node) * max_nodes);
    snake->buffered_dir = LEFT;
    snake->move_dir = LEFT;
    snake->length = 2;

    //tail
    snake->nodes[0].x_pos = start_x;
    snake->nodes[0].y_pos = start_y;

    //head
    snake->nodes[1].x_pos = start_x - 1;
    snake->nodes[1].y_pos = start_y;
    
    return 0;
}

GpStatus render_pause (GameState* state, HWND hwnd) {
    //TODO:: Implement on pause action
    //Use existing buffer
    //Draw translucent black over it
    //Draw text saying "PAUSED"

    return Ok;
}

GpStatus render_game(GameState* state, HWND hwnd) {
    state->n_renders++;
    int gdi_status = 0;
    GpStatus status = Ok;
    
    //Setup memory HDC to draw into
    HDC hdc_win = GetDC(hwnd);
    HBITMAP h_bmap = CreateCompatibleBitmap(hdc_win, state->window_size.x, state->window_size.y);
    HDC hdc_mem = CreateCompatibleDC(hdc_win);
    HBITMAP old_bmap = SelectObject(hdc_mem, h_bmap); //swap in bitmap

    //Draw image
    GpGraphics* graphics;
    GdipCreateFromHDC(hdc_mem, &graphics);
    GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);

    GpSolidFill* solidfill;
    status = GdipCreateSolidFill(make_ARGB(190, 190, 190, 255), &solidfill);
    status = GdipFillRectangleI(graphics, solidfill, 0, 0, state->window_size.x, state->window_size.y);

    GpRect img_rect = {};
    GdipGetImageWidth(state->spritesheet, &(img_rect.Width));
    GdipGetImageHeight(state->spritesheet, &(img_rect.Height));

    GpRect dst_rect = {};
    dst_rect.Width = img_rect.Width * 5;
    dst_rect.Height = img_rect.Height * 5;

    img_rect.X = 0;
    img_rect.Y = 0;
    status = DrawImageRectangle(graphics, state->spritesheet, &dst_rect, &img_rect);

    //Save bitmap into game state buffer
    h_bmap = SelectObject(hdc_mem, old_bmap); //swap out bitmap
    HPALETTE h_pal =  GetCurrentObject(hdc_win, OBJ_PAL);
    
    GpBitmap* bitmap;
    status = GdipCreateBitmapFromHBITMAP(h_bmap, h_pal, &bitmap);

    //Clear old frame
    swap_buffers(&(state->drawbuffer), &bitmap);
    if (bitmap) {
        GdipDisposeImage(state->drawbuffer);
    }

    //Cache new frame
    // GpGraphics* graphics_win;
    // GdipCreateFromHDC(hdc_win, &graphics_win);
    // status = GdipCreateCachedBitmap( bitmap, graphics_win, &state->drawbuffer );

    //CLEANUP
    GdipDeleteGraphics(graphics);
    GdipDeleteBrush(solidfill);

    DeleteObject(h_bmap);
    DeleteDC(hdc_mem);

    return Ok;
}

int parse_cmdline(CmdConfig* config) {
    //TODO:: Set window size based on command line
    //uses __argv, __argc, feels like cheating

    config->window_x = WIDTH;
    config->window_y = HEIGHT;
    config->board_x = BOARD_WIDTH;
    config->board_y = BOARD_HEIGHT;
    
    return 0;
}

int setup_graphics(GameState* state, HWND hwnd) {
    GpStatus gp_status = GdipLoadImageFromFile(L"../assets/spritesheet.bmp", &(state->spritesheet));
    if (gp_status != Ok) {
        return gp_status;
    }

    render_game(state, hwnd);
    return Ok;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    //GDI+ init
    GdiplusStartupInput gdip_input;
    gdip_input.GdiplusVersion = 1; // Required for proper gdip startup.
    GdiplusStartupOutput gdip_output;
    ULONG_PTR gdip_token;
    GpStatus gp_status;
    int setup_status;

    gp_status = GdiplusStartup(&gdip_token, &gdip_input, &gdip_output);
    if (gp_status != Ok) {
        return gp_status;
    }
    
    CmdConfig config;
    setup_status = parse_cmdline(&config);
    if (setup_status) {
        return setup_status;
    }
    
    //Setup game state
    GameState state;
    setup_status = setup_game(&state, &config);
    if (setup_status != 0) {
        return setup_status;
    }

    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Snake Main Window";
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Snake",    // Window text
        //WS_OVERLAPPEDWINDOW,            // Window style
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,    // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // dwStyle, X, Y, nWidth, nHeight
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        &state        // Additional application data
        );

    if (hwnd == NULL) {
        return 0;
    }

    SetWindowPos(
        hwnd,          //HWND hWnd,
        HWND_TOP,           //HWND hWndInsertAfter,
        0, 0,         //int  X,Y,
        state.window_size.x, state.window_size.y,            //int  cx,cy,
        SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE         //UINT uFlags
        );

    ShowWindow(hwnd, nCmdShow);

    setup_status = setup_graphics(&state, hwnd);
    if (setup_status != 0) {
        return setup_status;
    }

    //Game loop start
    MSG msg;
    while(state.running) {

        //Process message queue
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {  // Change to PeekMessage to make non-blocking
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //handle pause evt

        //every x frames:
            //Move stuff
            //Handle collisions
            //Lose screen?
            //re-render
        
        //Redraw Window
        // InvalidateRgn(hwnd, NULL, FALSE);
        // UpdateWindow(hwnd);
        
        //Set timer to remaining time for vsync (Use SetTimer winapi to trigger an event?)
        Sleep(20);
        
    }

    PostQuitMessage(0);

    //cleanup (not needed, just good practice)
    free(state.game_snake.nodes);
    GdiplusShutdown(gdip_token);

    return 0;       
}
