#include "Registry.h"
#include "TextureCache.h"


struct WorldObject {
    Vector2 position;
    std::string assetPath;
};

std::vector<WorldObject> createWorld() {
    return {
        // Golden chest - center of the scene
        {{ 0,   0 },    "game/assets/Rocks and Chest/Chest/GoldenChest/1.png"},
        {{ 60,  -30 },  "game/assets/Rocks and Chest/Chest/GoldenChest/2.png"},

        // Iron chests nearby
        {{ -80, 40 },   "game/assets/Rocks and Chest/Chest/IronChest/1.png"},
        {{ 100, 50 },   "game/assets/Rocks and Chest/Chest/IronChest/2.png"},

        // Trees surrounding the chests
        {{ -120, -80 }, "game/assets/Trees/Tree1.png"},
        {{ 80,  -100 }, "game/assets/Trees/Tree2.png"},
        {{ -60, -120 }, "game/assets/Trees/Tree3.png"},
        {{ 140, -60 },  "game/assets/Trees/Tree1.png"},
        {{ -140, 60 },  "game/assets/Trees/Tree2.png"},
        {{ 120,  100 }, "game/assets/Trees/Tree3.png"},
        {{ -100, 120 }, "game/assets/Trees/Tree1.png"},
        {{ 60,   130 }, "game/assets/Trees/Branch.png"},
        {{ -30,  140 }, "game/assets/Trees/Branch.png"},

        // Rocks scattered close by
        {{ -60,  60 },  "game/assets/Rocks and Chest/Rocks/Rock1.png"},
        {{ 80,   20 },  "game/assets/Rocks and Chest/Rocks/Rock2.png"},
        {{ -80, -40 },  "game/assets/Rocks and Chest/Rocks/Rock3.png"},
        {{ 40,   80 },  "game/assets/Rocks and Chest/Rocks/Rock4.png"},
        {{ -40, -80 },  "game/assets/Rocks and Chest/Rocks/Rock5.png"},
        {{ 120,  -20 }, "game/assets/Rocks and Chest/Rocks/Rock6.png"},

        // Magic stones in a tight arc around the chest
        {{ -160, 0 },   "game/assets/Rocks and Chest/MagicStoons/MagicStones1.png"},
        {{ -120, -100 },"game/assets/Rocks and Chest/MagicStoons/MagicStones2.png"},
        {{ 0,   -150 }, "game/assets/Rocks and Chest/MagicStoons/MagicStones3.png"},
        {{ 120, -100 }, "game/assets/Rocks and Chest/MagicStoons/MagicStones4.png"},
        {{ 160,  0 },   "game/assets/Rocks and Chest/MagicStoons/MagicStones5.png"},
        {{ 100,  130 }, "game/assets/Rocks and Chest/MagicStoons/MagicStones6.png"},

        // Lamp and sign as entrance markers
        {{ -20,  170 }, "game/assets/Wooden/Lamp.png"},
        {{ 30,   170 }, "game/assets/Wooden/Sign.PNG"},
    };
}

int main() {
    InitWindow(768, 432, "Asset Registry Demo");
    SetTargetFPS(60);
    const std::string assetsAbsolutePath = "C:/Users/Bella/CLionProjects/AssetRegistry/";
    const Vector2 WORLD_ORIGIN = { 768/2.0f, 432/2.0f };

    try {
        Registry registry;
        TextureCache textureCache(registry);

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(LIME);
            // Texture2D tex = textureCache.get("C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/tree.png");
            //DrawTexture(tex, 100, 100, WHITE);
            for (const auto& item : createWorld()) {
                const Texture2D tex = textureCache.get(assetsAbsolutePath + item.assetPath);
                DrawTexture(tex, item.position.x + WORLD_ORIGIN.x, item.position.y + WORLD_ORIGIN.y, WHITE);
            }
            EndDrawing();
        }
    } catch (const CacheError& e) {
        TraceLog(LOG_ERROR, e.what());
    }

    CloseWindow();
}
