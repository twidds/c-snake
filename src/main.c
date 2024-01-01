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


enum object_type {
    INVALID,
    APPLE,
    SNAKE,
    SNAKE_HEAD,
    SNAKE_TAIL,
    SNAKE_BODY,
    TYPE_END
};

typedef struct node {
    enum object_type m_type;
    int pos_x;
    int pos_y;
} node;

typedef struct snake {
    enum object_type m_type;
    int length;
    struct node* m_nodes; //points at head
} snake;

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

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
    

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Snake",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        //WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;       
}
