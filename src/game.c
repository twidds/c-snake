#include <time.h>
#include <stdio.h>

#ifndef UNICODE
    #define UNICODE
#endif
#include <windows.h>
#include "game.h"

//TODO:: Make size dynamic, use these for min sizes.
#define MIN_BOARD_WIDTH 10
#define MIN_BOARD_HEIGHT 10

#define MIN_WINDOW_WIDTH 100
#define MIN_WINDOW_HEIGHT 100

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define N_TRACKED_FRAMES 10

//This shouldn't be hardcoded.... but I'm lazy right now.
//TODO:: Create some sort of metadata file packaged with spritesheet instead.
typedef enum SnakeSpriteIndices {
    SNAKE_SPRITE_HEAD_LR,
    SNAKE_SPRITE_HEAD_DU,
    SNAKE_SPRITE_BODY_LR,
    SNAKE_SPRITE_BODY_UD,
    SNAKE_SPRITE_BODY_LU,
    SNAKE_SPRITE_BODY_RU,
    SNAKE_SPRITE_BODY_RD,
    SNAKE_SPRITE_BODY_LD,
    SNAKE_SPRITE_TAIL_RL,
    SNAKE_SPRITE_TAIL_DU,
    SNAKE_SPRITE_UNUSED_10,
    SNAKE_SPRITE_UNUSED_11,
    SNAKE_SPRITE_CHERRY,
    SNAKE_SPRITE_UNUSED_13,
    SNAKE_SPRITE_UNUSED_14,
    SNAKE_SPRITE_UNUSED_15,
    SNAKE_SPRITE_GROUND_BROWN,
    SNAKE_SPRITE_UNUSED_17,
    SNAKE_SPRITE_UNUSED_18,
    SNAKE_SPRITE_UNUSED_19,
    SNAKE_SPRITE_COUNT
} SnakeSpriteIndices;

const char* STAT_STRINGS[STAT_COUNT] = {
    "# renders: %d", 
    "# draws: %d", 
    "fps: %d", 
    "frame ms: %d",
    "msg ms: %d", 
    "paint ms: %d",
    "render ms: %d",
    };

int parse_cmdline(CmdConfig* config, char** argv, int argc) {
    //TODO:: Set window size based on command line
    //uses __argv, __argc, feels like cheating

    config->window_size.x = WINDOW_WIDTH;
    config->window_size.y = WINDOW_HEIGHT;
    config->grid_size.x = BOARD_WIDTH;
    config->grid_size.y = BOARD_HEIGHT;
    
    if (config->window_size.x < MIN_WINDOW_WIDTH || config->window_size.y < MIN_WINDOW_HEIGHT) {
        return 1;
    }

    if (config->grid_size.x < MIN_BOARD_WIDTH || config->grid_size.y < MIN_BOARD_HEIGHT) {
        return 2;
    }
    
    return 0;
}


void delete_fpstracker(FpsTimer* tracker) {
    free(tracker->frame_times);
}

void init_fpstracker(FpsTimer* tracker , int n_trackedframes) {
    if (tracker->frame_times) {
        delete_fpstracker(tracker);
    }

    tracker->cur_fps = 0;
    tracker->cur_index = 0;
    tracker->n_trackedframes = n_trackedframes;
    tracker->frame_times = malloc(n_trackedframes * sizeof(clock_t));
}

void update_fpstracker(FpsTimer* tracker, clock_t latest) {
    tracker->cur_index++;
    if (tracker->cur_index == tracker->n_trackedframes) {
        tracker->cur_index = 0;
    }

    tracker->frame_times[tracker->cur_index] = latest;
    clock_t sum = 0;
    for (int i = 0; i < tracker->n_trackedframes; i++) {
        sum += tracker->frame_times[i];
    }

    if (sum == 0) {
        tracker->cur_fps = 0;
    } else {
        tracker->cur_fps = CLOCKS_PER_SEC * tracker->n_trackedframes / sum;
    }
}

//sets bufa to bufb, then bufb to old bufa
void swap_buffers(void** bufa, void** bufb) {
    void* tmp = *bufa;
    *bufa = *bufb;
    *bufb = tmp;
}

void realloc_snake_nodes(Snake* snake, int newsize) {
    // int newmax = 2 * snake->max_length;
    Node* new_nodes = malloc(sizeof(Node) * newsize);
    
    if (snake->nodes){
        int newlength = newsize < snake->length ? newsize : snake->length;
        memcpy(new_nodes, snake->nodes, sizeof(Node) * newlength);
        free(snake->nodes);
        snake->length = newlength;
    }

    snake->nodes = new_nodes;
    snake->max_length = newsize;
}

int setup_game(GameState* state, CmdConfig* config) {
    //used for timing stats
    QueryPerformanceFrequency(&state->clock_freq);
    init_fpstracker(&state->fps_timer, N_TRACKED_FRAMES);

    //from cmd line
    state->window_size = config->window_size;
    state->grid_size = config->grid_size;
    
    //game initial state
    state->game_flags[FLAG_RUNNING] = true;
    state->game_flags[FLAG_PAUSED] = false;
    state->game_flags[FLAG_SHOW_STATS] = true;
    
    //setup snake
    Snake* snake = &(state->game_snake);
    int start_x = state->grid_size.x/2;
    int start_y = state->grid_size.y/2;
    snake->move_dir = DIR_LEFT;
    snake->buffered_dir = DIR_LEFT;

    int max_nodes = 10;
    realloc_snake_nodes(snake, max_nodes);
    snake->length = 2;

    //tail
    snake->nodes[0].position.x = start_x;
    snake->nodes[0].position.y = start_y;

    //head
    snake->nodes[1].position.x = start_x - 1;
    snake->nodes[1].position.y = start_y;

    //Main Window
    state->main_window = setup_window(
        L"Snake Main Window", //Window name
        L"Snake", //Window text
        state, //Game data
        state->window_size.x, state->window_size.y //Window x/y size
        );
    if (state->main_window == NULL) {
        return 1;
    }

    //Graphics
    GrStatus graphics_status = setup_graphics(&state->graphics_data, state->main_window);
    if (graphics_status != GRAPHICS_OK) {
        return 1;
    }

    return 0;
}

LRESULT game_exit(GameState* state) {
    state->game_flags[FLAG_RUNNING] = false;
    return 0;
}

void cleanup_game(GameState* state) {
    cleanup_graphics(&state->graphics_data);
    DestroyWindow(state->main_window);
    // PostQuitMessage(0); //Needed? About to exit anyway
    free(state->game_snake.nodes);
}

//Renders game world to bitmap buffer, which will later be drawn.
//TODO:: this shouldn't rely on hwnd, should be able to create proper bitmap without that...
//TODO:: remove gdi code from here, only use gdip
    //GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(INT width, INT height, INT stride, PixelFormat format, BYTE* scan0, GpBitmap** bitmap)
    //Probably need a function to create a proper bitmap to use here.
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

    GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor); //NO SMOOTH MA IMAGE!
    GdipSetPixelOffsetMode(graphics, PixelOffsetModeHalf); //NO CUT MA PIXELS IN HALF!

    GpSolidFill* solidfill;
    status = GdipCreateSolidFill(CreateARGB(190, 190, 190, 255), &solidfill);
    status = GdipFillRectangleI(graphics, solidfill, 0, 0, state->window_size.x, state->window_size.y);

    GpRect dst_rect = {};
    dst_rect.X = 100;
    dst_rect.Y = 100;
    dst_rect.Width = 500;
    dst_rect.Height = 500;

    // DrawSprite_Index(graphics, &dst_rect, state->graphics_data.spritesheet, SNAKE_SPRITE_HEAD_LR);
    int index = state->game_stats[STAT_N_RENDERS] / 10;
    index = index % SNAKE_SPRITE_COUNT;
    DrawSprite_Index(graphics, &dst_rect, state->graphics_data.spritesheet, index);

    // img_rect.X = 0;
    // img_rect.Y = 0;
    // status = DrawImageRectangle(graphics, state->graphics_data.spritesheet, &dst_rect, &img_rect);

    //draw stats
    RECT text_rect = {0, 0, state->window_size.x, state->window_size.y}; // LEFT,TOP,RIGHT,BOT
    if (state->game_flags[FLAG_SHOW_STATS]) {
        char statstring[30];
        #ifdef UNICODE
            wchar_t statstringW[30];
        #endif
        
        for (int i = 0; i < STAT_COUNT; i++) {
            sprintf(statstring, STAT_STRINGS[i], state->game_stats[i]);
            text_rect.top += DrawTextA(hdc_mem, statstring, -1, &text_rect, DT_TOP | DT_LEFT);
            // #ifdef UNICODE
            //     MultiByteToWideChar(CP_UTF8, 0, statstring, -1, statstringW, 30);
            //     text_rect.top += DrawText(hdc_mem, statstringW, -1, &text_rect, DT_TOP | DT_LEFT);
            // #else
            //     text_rect.top += DrawText(hdc_mem, statstring, -1, &text_rect, DT_TOP | DT_LEFT);
            // #endif
        }
    }

    //Save bitmap into game state buffer
    h_bmap = SelectObject(hdc_mem, old_bmap); //swap out bitmap
    HPALETTE h_pal =  GetCurrentObject(hdc_win, OBJ_PAL);
    
    GpBitmap* bitmap;
    status = GdipCreateBitmapFromHBITMAP(h_bmap, h_pal, &bitmap);

    //Clear old frame
    swap_buffers(&(state->graphics_data.render_buffer), &bitmap);
    if (bitmap) {
        GdipDisposeImage(bitmap);
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

GpStatus render_pause (GameState* state, HWND hwnd) {
    //TODO:: Implement on pause action
    //Use existing buffer
    //Draw translucent black over it
    //Draw text saying "PAUSED"

    return Ok;
}


//Paints bitmap buffer to screen
LRESULT paint_window(HWND hwnd, GameState* state) {
    LARGE_INTEGER paint_start; LARGE_INTEGER paint_end;
    QueryPerformanceCounter(&paint_start);
    GpStatus gp_status;

    //Put draw from cached bitmap here:
    state->game_stats[STAT_N_DRAWS]++;
    PAINTSTRUCT ps;
    // RECT rc;
    InvalidateRgn(hwnd, NULL, FALSE);
    // GetClientRect(hwnd, &rc);
    HDC hdc = BeginPaint(hwnd, &ps);

    GpGraphics* graphics;
    gp_status = GdipCreateFromHDC(hdc, &graphics);
    gp_status = GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);

    // gp_status = GdipDrawCachedBitmap(graphics, state->drawbuffer, 0, 0);
    gp_status = GdipDrawImage(graphics, state->graphics_data.render_buffer, 0, 0);
    
    EndPaint(hwnd, &ps);
    GdipDeleteGraphics(graphics);

    QueryPerformanceCounter(&paint_end);
    state->game_stats[STAT_PAINT_MS] = 1000 * (paint_end.QuadPart - paint_start.QuadPart) / state->clock_freq.QuadPart;

    return 0;
}


LRESULT handle_keys(HWND hwnd, GameState* state, WPARAM wParam, LPARAM lParam) {
    
    switch(wParam) { //special keys
        case KEYINPUT_ESCAPE: //exit
            state->game_flags[FLAG_RUNNING] = false;
            break;
        case 'P': //pause
            state->game_flags[FLAG_PAUSED] = !(state->game_flags[FLAG_PAUSED]);
            state->game_flags[FLAG_PAUSING] = state->game_flags[FLAG_PAUSED];
            break;
        case 'S': //stats
            state->game_flags[FLAG_SHOW_STATS] = !(state->game_flags[FLAG_SHOW_STATS]);
            break;
    }

    if (state->game_flags[FLAG_PAUSED]) { //skip rest of keys
        return 0;
    }
    
    switch(wParam) { //gameplay keys
        case KEYINPUT_LEFT:
            if (state->game_snake.move_dir != DIR_RIGHT) {
                state->game_snake.buffered_dir = DIR_LEFT;
            }
            break;
        case KEYINPUT_UP:
            if (state->game_snake.move_dir != DIR_DOWN) {
                state->game_snake.buffered_dir = DIR_UP;
            }
            break;
        case KEYINPUT_RIGHT:
            if (state->game_snake.move_dir != DIR_LEFT) {
                state->game_snake.buffered_dir = DIR_RIGHT;
            }
            break;
        case KEYINPUT_DOWN:
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
            return game_exit(state);
        case WM_PAINT:
            return paint_window(hwnd, state);
        case WM_KEYDOWN:
            return handle_keys(hwnd, state, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND setup_window(const wchar_t* WindowName, const wchar_t* WindowText, void* AppData, int x_size, int y_size) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Register the window class.
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = WindowName;
    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        WindowName,                     // Window class
        WindowText,    // Window text
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,    // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // dwStyle, X, Y, nWidth, nHeight
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        AppData        // Additional application data
        );

    if (hwnd == NULL) {
        return NULL;
    }

    SetWindowPos(
        hwnd,          //HWND hWnd,
        HWND_TOP,           //HWND hWndInsertAfter,
        0, 0,         //int  X,Y,
        x_size, y_size,            //int  cx,cy,
        SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE         //UINT uFlags
        );

    ShowWindow(hwnd, SW_NORMAL);

    return hwnd;
}

void run_game(GameState* state) {
    GpStatus status = render_game(state, state->main_window); //initial draw
    
    MSG msg;
    while(state->game_flags[FLAG_RUNNING]) {
        // LARGE_INTEGER f_start; LARGE_INTEGER f_end;
        LARGE_INTEGER msg_start; LARGE_INTEGER msg_end;
        LARGE_INTEGER rndr_start; LARGE_INTEGER rndr_end;
        clock_t f_start = clock();


        //Process message queue
        QueryPerformanceCounter(&msg_start);
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {  // Change to PeekMessage to make non-blocking
            //Should ignore draw calls here?
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        QueryPerformanceCounter(&msg_end);
        state->game_stats[STAT_MSG_MS] = 1000 * (msg_end.QuadPart - msg_start.QuadPart) / state->clock_freq.QuadPart;

        //handle pause evt

        //every x frames:
            //Move stuff
            //Handle collisions
            //Lose screen?
            //re-render
        
        QueryPerformanceCounter(&rndr_start);
        render_game(state, state->main_window);
        QueryPerformanceCounter(&rndr_end);
        state->game_stats[STAT_RENDER_MS] = 1000 * (rndr_end.QuadPart - rndr_start.QuadPart) / state->clock_freq.QuadPart;

        //Redraw Window
        InvalidateRgn(state->main_window, NULL, FALSE);
        UpdateWindow(state->main_window);
        
        //Set timer to remaining time for vsync (Use SetTimer winapi to trigger an event?)
        Sleep(100);

        clock_t f_end = clock();
        update_fpstracker(&state->fps_timer, f_end - f_start);
        state->game_stats[STAT_FPS] = state->fps_timer.cur_fps;
        // state.game_stats[STAT_FRAME_MS] = 1000 * (f_end - f_start)/CLOCKS_PER_SEC;
        state->game_stats[STAT_FRAME_MS] = f_end - f_start;
    }
}