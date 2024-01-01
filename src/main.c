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
    wchar_t* render_buffer;
    snake game_snake;
    node apple;
} game_state;

void setconsole(int w, int h);

void clearconsole();

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
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                // All painting occurs here, between BeginPaint and EndPaint.
                

                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

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
    state->render_buffer = malloc(WIDTH * HEIGHT * sizeof(wchar_t));
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

    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Snake Main Window";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    
    game_state state = {};
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
    free(state.render_buffer);
    

    return 0;       
}
