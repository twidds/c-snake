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
//#pragma comment(lib, "gdiplus.lib")

//TODO:: Make size dynamic, use these for min sizes.
#define MIN_WIDTH 10
#define MIN_HEIGHT 10

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 50

#define WIDTH 800
#define HEIGHT 800


// typedef enum SnakeObjectType {
//     INVALID,
//     APPLE,
//     SNAKE,
//     SNAKE_HEAD,
//     SNAKE_TAIL,
//     SNAKE_BODY,
//     TYPE_END
// } SnakeObjectType;

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


typedef struct GameState {
    bool show_stats;
    int n_frames;
    bool paused;
    bool running;
    unsigned int score;
    // Direction move_dir;
    GpImage* spritesheet;
    // wchar_t* front_buffer;
    // wchar_t* back_buffer;
    Snake game_snake;
    Node apple;
} GameState;

void setconsole(int w, int h);

void clearconsole();

void swap_buffers(wchar_t** bufa, wchar_t** bufb) {
    wchar_t* tmp = *bufa;
    *bufa = *bufb;
    *bufb = tmp;
}

// void render(wchar_t* buffer, void* object){
//     enum SnakeObjectType* t = object;
//     switch(*t) {
//         case APPLE:
//             break;
//         case SNAKE:
//             break;
//         case SNAKE_HEAD:
//             break;
//         case SNAKE_BODY:
//             break;
//         case SNAKE_TAIL:
//             break;
//     }
// }


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


LRESULT PaintWindow(HWND hwnd, GameState* state) {
    // const wchar_t* message = L"WWWWWWWWWW";
    // const wchar_t* msg2 = L"\U000021D7 wwwwwwwwww";
    // printf("paint call\r\n");
    state->n_frames++;
    PAINTSTRUCT ps;
    RECT rc;
    InvalidateRgn(hwnd, NULL, FALSE);
    GetClientRect(hwnd, &rc);
    HDC hdc = BeginPaint(hwnd, &ps);

    GpGraphics* graphics;
    GdipCreateFromHDC(hdc, &graphics);
    GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);

    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
    //GdipDrawImage(graphics, state->test_image, 0, 0);
    //GdipDrawImageRect(graphics, state->test_image, 
    //    0, 0, //x, y
    //    50,50 //width, height
    //    );
    GpRect img_rect = {};
    GdipGetImageWidth(state->spritesheet, &(img_rect.Width));
    GdipGetImageHeight(state->spritesheet, &(img_rect.Height));

    //GpStatus WINGDIPAPI GdipDrawImagePoints(GpGraphics *graphics, GpImage *image, GDIPCONST GpPointF *dstpoints, INT count)
    GpRect dst_rect = {};
    dst_rect.Width = img_rect.Width * 5;
    dst_rect.Height = img_rect.Height * 5;

    img_rect.X = 0;
    img_rect.Y = 0;
    //img_rect.Width = 200;
    //img_rect.Height = 250;
    

    DrawImageRectangle(graphics, state->spritesheet, &dst_rect, &img_rect);

    // DrawRotatedImage(graphics, state->test_image, &dst_rect, &img_rect, Rotate270);

    
    //Debug
    
    if (state->show_stats) {
        char framecount[30];
        wchar_t framecountW[30];
        strcpy(framecount, "frames: ");
        int index = strlen(framecount);
        itoa(state->n_frames, &framecount[index], 10);
        MultiByteToWideChar(CP_UTF8, 0, framecount, -1, framecountW, 30);
        DrawText(hdc, framecountW, -1, &rc, DT_TOP | DT_LEFT);
    }
    
    EndPaint(hwnd, &ps);
return 0;

}

LRESULT HandleKeys(HWND hwnd, GameState* state, WPARAM wParam, LPARAM lParam) {
    
    switch(wParam) { //special keys
        case VK_ESCAPE: //exit
            state->running = false;
            break;
        case 'P': //pause
            state->paused = !(state->paused);
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

int setup_game(GameState* state) {
    
    //gp_status = GdipLoadImageFromFile(L"../assets/smiley.png", &test_image);
    GpStatus gp_status = GdipLoadImageFromFile(L"../assets/spritesheet.bmp", &(state->spritesheet));
    if (gp_status != Ok) {
        return gp_status;
    }
    
    state->running = true;
    state->paused = false;
    state->score = 0;
    state->n_frames = 0;
    state->show_stats = false;
    
    //setup snake
    int start_x = BOARD_WIDTH/2;
    int start_y = BOARD_HEIGHT/2;
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    //TODO:: Set window size based on command line
    //  __argv, __argc
    
    //GDI+ init
    GdiplusStartupInput gdip_input;
    gdip_input.GdiplusVersion = 1; // Required for proper gdip startup.
    GdiplusStartupOutput gdip_output;
    ULONG_PTR gdip_token;
    GpStatus gp_status;

    gp_status = GdiplusStartup(&gdip_token, &gdip_input, &gdip_output);
    if (gp_status != Ok) {
        return gp_status;
    }

    //Setup game state
    GpRect windowBounds = {0, 0, WIDTH, HEIGHT}; //X, Y, Width, Height
    GameState state = {};
    int setup_status = setup_game(&state);
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
        WIDTH, HEIGHT,            //int  cx,cy,
        SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE         //UINT uFlags
        );

    ShowWindow(hwnd, nCmdShow);

    //Game loop start
    MSG msg;
    while(state.running) {

        //Process message queue
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {  // Change to PeekMessage to make non-blocking
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //Render to some target buffer? Probably a global...
        
        //Redraw Window
        // InvalidateRgn(hwnd, NULL, FALSE);
        // UpdateWindow(hwnd);
        

        //Set timer to remaining time for vsync (Use SetTimer winapi to trigger an event?)
        Sleep(10);
        
    }

    PostQuitMessage(0);


    
    //Game loop end

    //cleanup (not needed, just good practice)
    free(state.game_snake.nodes);
    // free(state.front_buffer);
    // free(state.back_buffer);
    GdiplusShutdown(gdip_token);

    return 0;       
}
