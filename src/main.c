//Entry point for game.
//Actual game logic is all inside game.c

#ifdef _WIN32
    #include <windows.h>
#endif /* _WIN32 */

#include "game.h"

int main(char** argv, int argc) {
    CmdConfig config;
    if (parse_cmdline(&config, argv, argc) != 0){
        return 1;
    }

    GameState state = {};
    if (setup_game(&state, &config) != 0) {
        return 2;
    }
    
    run_game(&state);
    cleanup_game(&state);
    return 0;
}

#ifdef _WIN32
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
        return main(__argv, __argc);
    }
#endif /* _WIN32 */
