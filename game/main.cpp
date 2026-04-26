#include "raylib.h"

int main() {
    InitWindow(1280, 720, "Asset Registry Demo");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("raylib works!", 400, 300, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
}