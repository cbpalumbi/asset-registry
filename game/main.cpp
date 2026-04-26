#include "Registry.h"
#include "TextureCache.h"

int main() {
    InitWindow(1280, 720, "Asset Registry Demo");
    SetTargetFPS(60);

    try {
        Registry registry;
        TextureCache textureCache(registry);

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            Texture2D tex = textureCache.get("C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/tree.png");
            DrawTexture(tex, 100, 100, WHITE);
            EndDrawing();
        }
    } catch (const CacheError& e) {
        TraceLog(LOG_ERROR, e.what());
    }

    CloseWindow();
}