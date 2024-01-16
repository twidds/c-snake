//References for future stuffs:
    //  UTF-8 in WINAPI: https://stackoverflow.com/questions/8831143/windows-api-ansi-functions-and-utf-8
    //  https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    //  GetCommandLine -> GetCommandLineW

// //UNICODE: all winapi functions default to W instead of A
#ifndef UNICODE
    #define UNICODE
#endif
#include <windows.h>
#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>


// typedef HWND WindowHandle;
// typedef struct MainWindow {
//     WindowHandle handle;

// } MainWindow;

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

HWND setup_window(const wchar_t* WindowName, const wchar_t* WindowText, void* AppData, int x_size, int y_size) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Register the window class.
    // const wchar_t CLASS_NAME[]  = L;
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

int main(char** argv, int argc) {

    int setup_status;
    
    CmdConfig config;
    setup_status = parse_cmdline(&config, argv, argc);
    if (setup_status != 0) {
        return setup_status;
    }
    
    //Setup game state
    GameState state = {};
    setup_status = setup_game(&state, &config);
    if (setup_status != 0) {
        return setup_status;
    }

    HWND hwnd = setup_window(
        L"Snake Main Window", //Window name
        L"Snake", //Window text
        &state, //Game data
        state.window_size.x, state.window_size.y //Window x/y size
        );

    
    ULONG_PTR gdip_token;
    gdip_token = setup_graphics(&state.graphics_data, hwnd);
    if (gdip_token == 0) {
        return 1;
    }

    run_game(&state, hwnd);

    //cleanup (not needed, just good practice)
    free(state.game_snake.nodes);
    GdiplusShutdown(gdip_token);

    DestroyWindow(hwnd);

    PostQuitMessage(0); //Needed? About to exit anyway
    return 0;
}

//Can use for wrapper
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
    return main(__argv, __argc);
}
