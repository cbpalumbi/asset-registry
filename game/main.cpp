#include "Registry.h"
#include "TextureCache.h"

enum class PlayerDirection { Down, DownSide, Side, UpSide, Up };

struct Player {
    Vector2 position;
    PlayerDirection direction;
    bool moving;
    int currentFrame;
    float frameTimer;
    const float FRAME_DURATION = 0.1f;
    const float SPEED = 150.0f;
    bool facingLeft;
    std::string currentFolder;
    std::string lastMovingFolder = "game/assets/Rogue Animationset/Idle";
};

std::string getAnimationFolder(const Player& player) {
    if (!player.moving) return player.lastMovingFolder;
    switch (player.direction) {
        case PlayerDirection::Down:     return "game/assets/Rogue Animationset/DownWalk";
        case PlayerDirection::DownSide: return "game/assets/Rogue Animationset/DownSideWalk";
        case PlayerDirection::Side:     return "game/assets/Rogue Animationset/SideWalk";
        case PlayerDirection::UpSide:   return "game/assets/Rogue Animationset/UpSideWalk";
        case PlayerDirection::Up:       return "game/assets/Rogue Animationset/UpWalk";
        default:                        return "game/assets/Rogue Animationset/Idle";
    }
}

std::string getFramePath(const Player& player) {
    const std::string assetsAbsolutePath = "C:/Users/Bella/CLionProjects/AssetRegistry/";
    std::string folder = assetsAbsolutePath + getAnimationFolder(player);
    std::string frameName = folder.substr(folder.find_last_of('/') + 1);
    return folder + "/" + frameName + std::to_string(player.currentFrame + 1) + ".png";
}

void updatePlayer(Player& player, float dt) {
    Vector2 movement = { 0, 0 };

    bool up    = IsKeyDown(KEY_W);
    bool down  = IsKeyDown(KEY_S);
    bool left  = IsKeyDown(KEY_A);
    bool right = IsKeyDown(KEY_D);

    if (up && right)        { movement = { 1, -1 };  player.direction = PlayerDirection::UpSide;   player.facingLeft = false; }
    else if (up && left)    { movement = { -1, -1 }; player.direction = PlayerDirection::UpSide;   player.facingLeft = true;  }
    else if (down && right) { movement = { 1, 1 };   player.direction = PlayerDirection::DownSide; player.facingLeft = true;  }
    else if (down && left)  { movement = { -1, 1 };  player.direction = PlayerDirection::DownSide; player.facingLeft = false; }
    else if (up)            { movement = { 0, -1 };  player.direction = PlayerDirection::Up;        }
    else if (down)          { movement = { 0, 1 };   player.direction = PlayerDirection::Down;      }
    else if (right)         { movement = { 1, 0 };   player.direction = PlayerDirection::Side;      player.facingLeft = true;  }
    else if (left)          { movement = { -1, 0 };  player.direction = PlayerDirection::Side;      player.facingLeft = false; }

    player.moving = (movement.x != 0 || movement.y != 0);

    // Update lastMovingFolder before getAnimationFolder is called
    if (player.moving) {
        switch (player.direction) {
            case PlayerDirection::Down:     player.lastMovingFolder = "game/assets/Rogue Animationset/DownWalk"; break;
            case PlayerDirection::DownSide: player.lastMovingFolder = "game/assets/Rogue Animationset/DownSideWalk"; break;
            case PlayerDirection::Side:     player.lastMovingFolder = "game/assets/Rogue Animationset/SideWalk"; break;
            case PlayerDirection::UpSide:   player.lastMovingFolder = "game/assets/Rogue Animationset/UpSideWalk"; break;
            case PlayerDirection::Up:       player.lastMovingFolder = "game/assets/Rogue Animationset/UpWalk"; break;
        }
    }

    // Normalize diagonal movement
    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.707f;
        movement.y *= 0.707f;
    }

    player.position.x += movement.x * player.SPEED * dt;
    player.position.y += movement.y * player.SPEED * dt;

    // Reset frame only when animation folder changes
    std::string newFolder = getAnimationFolder(player);
    if (newFolder != player.currentFolder) {
        player.currentFolder = newFolder;
        player.currentFrame = 0;
        player.frameTimer = 0;
    }

    // Advance frame
    player.frameTimer += dt;
    if (player.frameTimer >= player.FRAME_DURATION) {
        player.frameTimer = 0;
        player.currentFrame = (player.currentFrame + 1) % 6;
    }
}

void drawPlayer(Player& player, TextureCache& textureCache, Vector2 worldOrigin) {
    Texture2D tex = textureCache.get(getFramePath(player));

    // Flip horizontally for left-facing directions
    if (player.facingLeft) {
        DrawTextureRec(tex,
            { 0, 0, -32, 32 }, // negative width flips the texture
            { player.position.x + worldOrigin.x, player.position.y + worldOrigin.y },
            WHITE);
    } else {
        DrawTexture(tex,
            player.position.x + worldOrigin.x,
            player.position.y + worldOrigin.y,
            WHITE);
    }
}

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
