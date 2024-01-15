//References for future stuffs:
    //  UTF-8 in WINAPI: https://stackoverflow.com/questions/8831143/windows-api-ansi-functions-and-utf-8
    //  https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    //  GetCommandLine -> GetCommandLineW

#include <stdio.h>
#include <stdbool.h>

//UNICODE: all winapi functions default to W instead of A
#ifndef UNICODE
    #define UNICODE
#endif
#include <windows.h>
#include <gdiplus.h>

//TODO:: Make size dynamic, use these for min sizes.
#define MIN_BOARD_WIDTH 10
#define MIN_BOARD_HEIGHT 10

#define MIN_WINDOW_WIDTH 100
#define MIN_WINDOW_HEIGHT 100

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800


typedef enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct Node {
    int x_pos;
    int y_pos;
} Node;

typedef struct Snake {
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

typedef enum GameFlag {
    FLAG_RUNNING,
    FLAG_PAUSED,
    FLAG_PAUSING,
    FLAG_SHOW_STATS,
    FLAGS_COUNT
} GameFlag;

typedef enum GameStats {
    STAT_N_RENDERS,
    STAT_N_DRAWS,
    STAT_FPS,
    STAT_COUNT
} GameStats;
const char* stat_strings[STAT_COUNT] = {"# renders: %d", "# draws: %d", "fps: %d"};

typedef struct GameState {
    bool game_flags[FLAGS_COUNT];
    int game_stats[STAT_COUNT];
    int score;

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


LRESULT PaintWindow(HWND hwnd, GameState* state) {
    GpStatus gp_status;

    //Put draw from cached bitmap here:
    state->game_stats[STAT_N_DRAWS]++;
    PAINTSTRUCT ps;
    RECT rc;
    InvalidateRgn(hwnd, NULL, FALSE);
    GetClientRect(hwnd, &rc);
    HDC hdc = BeginPaint(hwnd, &ps);

    GpGraphics* graphics;
    gp_status = GdipCreateFromHDC(hdc, &graphics);
    gp_status = GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);

    // gp_status = GdipDrawCachedBitmap(graphics, state->drawbuffer, 0, 0);
    gp_status = GdipDrawImage(graphics, state->drawbuffer, 0, 0);

    //draw stats
    if (state->game_flags[FLAG_SHOW_STATS]) {
        char statstring[30];
        wchar_t statstringW[30];
        
        for (int i = 0; i < STAT_COUNT; i++) {
            sprintf(statstring, stat_strings[i], state->game_stats[i]);
            MultiByteToWideChar(CP_UTF8, 0, statstring, -1, statstringW, 30);
            rc.top += DrawText(hdc, statstringW, -1, &rc, DT_TOP | DT_LEFT);    
        }
    }
    
    EndPaint(hwnd, &ps);
    GdipDeleteGraphics(graphics);
    return 0;
}

LRESULT HandleKeys(HWND hwnd, GameState* state, WPARAM wParam, LPARAM lParam) {
    
    switch(wParam) { //special keys
        case VK_ESCAPE: //exit
            state->game_flags[FLAG_RUNNING] = false;
            break;
        case 'P': //pause
            state->game_stats[FLAG_PAUSED] = !(state->game_stats[FLAG_PAUSED]);
            state->game_stats[FLAG_PAUSING] = state->game_stats[FLAG_PAUSED];
            break;
        case 'S': //stats
            state->game_stats[FLAG_SHOW_STATS] = !(state->game_stats[FLAG_SHOW_STATS]);
            break;
    }

    if (state->game_flags[FLAG_PAUSED]) { //skip rest of keys
        return 0;
    }
    
    switch(wParam) { //gameplay keys
        case VK_LEFT:
            if (state->game_snake.move_dir != DIR_RIGHT) {
                state->game_snake.buffered_dir = DIR_LEFT;
            }
            break;
        case VK_UP:
            if (state->game_snake.move_dir != DIR_DOWN) {
                state->game_snake.buffered_dir = DIR_UP;
            }
            break;
        case VK_RIGHT:
            if (state->game_snake.move_dir != DIR_LEFT) {
                state->game_snake.buffered_dir = DIR_RIGHT;
            }
            break;
        case VK_DOWN:
            if (state->game_snake.move_dir != DIR_UP) {
                state->game_snake.buffered_dir = DIR_DOWN;
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
            state->game_flags[FLAG_RUNNING] = false;
            return 0;
        case WM_PAINT:
            return PaintWindow(hwnd, state);
        case WM_KEYDOWN:
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
    //from cmd line
    state->window_size.x = config->window_x;
    state->window_size.y = config->window_y;
    state->board_size.x = config->board_x;
    state->board_size.y = config->board_y;
    
    //game initial state
    state->game_flags[FLAG_RUNNING] = true;
    state->game_flags[FLAG_PAUSED] = false;
    state->game_flags[FLAG_SHOW_STATS] = true;

    //Shouldn't be needed, memory is zeroed
    // state->game_stats[STAT_SCORE] = 0;
    // state->game_stats[STAT_N_DRAWS] = 0;
    // state->game_stats[STAT_N_RENDERS] = 0;
    
    //setup snake
    int start_x = state->board_size.x/2;
    int start_y = state->board_size.y/2;
    int max_nodes = 10;

    Snake* snake = &(state->game_snake);
    snake->max_length = max_nodes;
    snake->nodes = malloc(sizeof(Node) * max_nodes);
    snake->buffered_dir = DIR_LEFT;
    snake->move_dir = DIR_LEFT;
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
    // state->n_renders++;
    state->game_stats[STAT_N_RENDERS]++;
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

    config->window_x = WINDOW_WIDTH;
    config->window_y = WINDOW_HEIGHT;
    config->board_x = BOARD_WIDTH;
    config->board_y = BOARD_HEIGHT;
    
    if (config->window_x < MIN_WINDOW_WIDTH || config->window_y < MIN_WINDOW_HEIGHT) {
        return 1;
    }

    if (config->board_x < MIN_BOARD_WIDTH || config->board_y < MIN_BOARD_HEIGHT) {
        return 2;
    }
    
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
    
    CmdConfig config;
    setup_status = parse_cmdline(&config);
    if (setup_status) {
        return setup_status;
    }
    
    //Setup game state
    GameState state = {};
    setup_status = setup_game(&state, &config);
    if (setup_status != 0) {
        return setup_status;
    }

    gp_status = GdiplusStartup(&gdip_token, &gdip_input, &gdip_output);
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

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Snake",    // Window text
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
    while(state.game_flags[FLAG_RUNNING]) {

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
