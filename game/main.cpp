#include "Player.h"
#include "Registry.h"
#include "TextureCache.h"
#include "World.h"

int main() {
    InitWindow(768, 432, "Asset Registry Demo");
    SetTargetFPS(60);
    const std::string assetsAbsolutePath = "C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/";
    constexpr Vector2 WORLD_ORIGIN = { 768/2.0f, 432/2.0f };

    Player player = {};
    player.position = { 0, 0 };
    player.direction = PlayerDirection::Down;
    player.moving = false;
    player.currentFrame = 0;
    player.frameTimer = 0;
    player.facingLeft = false;

    try {
        Registry registry;
        TextureCache textureCache(registry);

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            updatePlayer(player, dt);

            BeginDrawing();
            ClearBackground(LIME);
            // Texture2D tex = textureCache.get("C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/tree.png");
            //DrawTexture(tex, 100, 100, WHITE);
            for (const auto& item : createWorld()) {
                const Texture2D tex = textureCache.get(assetsAbsolutePath + item.assetPath);
                DrawTexture(tex, item.position.x + WORLD_ORIGIN.x, item.position.y + WORLD_ORIGIN.y, WHITE);
            }

            drawPlayer(player, textureCache, WORLD_ORIGIN);

            EndDrawing();
        }
    }  catch (const CacheError& e) {
        TraceLog(LOG_ERROR, "CacheError: %s", e.what());
        DrawText(e.what(), 10, 50, 18, RED);

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(e.what(), 10, 200, 18, RED);
            EndDrawing();
        }
    }

    CloseWindow();
}
