#include "raylib.h"


int main(char** argv, int argc) {
    const int screen_w = 800;
    const int screen_h = 450;

    InitWindow(screen_w,screen_h, "raylib basic window");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Made our first window",190,200,20,LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;

}

