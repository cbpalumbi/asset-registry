#include "Player.h"
#include "Registry.h"
#include "TextureCache.h"
#include "World.h"
#include <cmath>
#include <unordered_map>
#include <string>

struct Frustum {
    Vector2 origin;
    Vector2 direction;
    float halfAngle;
    float range;
};

Vector2 getPlayerFacingDirection(const Player& player) {
    switch (player.direction) {
        case PlayerDirection::Up:       return {  0, -1 };
        case PlayerDirection::Down:     return {  0,  1 };
        case PlayerDirection::Side:     return { player.facingLeft ? 1.0f : -1.0f, 0 };
        case PlayerDirection::UpSide:   return { player.facingLeft ? 0.707f : -0.707f, -0.707f };
        case PlayerDirection::DownSide: return { player.facingLeft ? 0.707f : -0.707f,  0.707f };
        default:                        return {  0,  1 };
    }
}

bool isInFrustum(const Frustum& frustum, Vector2 objectPos) {
    Vector2 toObject = {
        objectPos.x - frustum.origin.x,
        objectPos.y - frustum.origin.y
    };
    float dist = sqrtf(toObject.x * toObject.x + toObject.y * toObject.y);
    if (dist < 0.001f) return true;
    if (dist > frustum.range) return false;

    Vector2 toObjectNorm = { toObject.x / dist, toObject.y / dist };
    float dot = frustum.direction.x * toObjectNorm.x + frustum.direction.y * toObjectNorm.y;
    return dot >= cosf(frustum.halfAngle);
}

void drawFrustum(const Frustum& frustum, Vector2 worldOrigin) {
    Vector2 screenOrigin = {
        frustum.origin.x + worldOrigin.x,
        frustum.origin.y + worldOrigin.y
    };

    // Angle of the facing direction
    float baseAngle = atan2f(frustum.direction.y, frustum.direction.x);

    // The two cone edge endpoints
    float leftAngle  = baseAngle - frustum.halfAngle;
    float rightAngle = baseAngle + frustum.halfAngle;

    Vector2 leftEdge  = { screenOrigin.x + cosf(leftAngle)  * frustum.range,
                          screenOrigin.y + sinf(leftAngle)  * frustum.range };
    Vector2 rightEdge = { screenOrigin.x + cosf(rightAngle) * frustum.range,
                          screenOrigin.y + sinf(rightAngle) * frustum.range };

    Color coneColor = { 255, 255, 0, 80 };  // translucent yellow

    // Filled triangle fan approximating the cone
    int arcSegments = 20;
    for (int i = 0; i < arcSegments; i++) {
        float t0 = (float)i       / arcSegments;
        float t1 = (float)(i + 1) / arcSegments;
        float a0 = leftAngle + t0 * (rightAngle - leftAngle);
        float a1 = leftAngle + t1 * (rightAngle - leftAngle);

        Vector2 p0 = { screenOrigin.x + cosf(a0) * frustum.range,
                       screenOrigin.y + sinf(a0) * frustum.range };
        Vector2 p1 = { screenOrigin.x + cosf(a1) * frustum.range,
                       screenOrigin.y + sinf(a1) * frustum.range };

        DrawTriangle(screenOrigin, p0, p1, coneColor);
    }

    // Outline
    DrawLineV(screenOrigin, leftEdge,  YELLOW);
    DrawLineV(screenOrigin, rightEdge, YELLOW);

    // Arc along the far edge
    for (int i = 0; i < arcSegments; i++) {
        float t0 = (float)i       / arcSegments;
        float t1 = (float)(i + 1) / arcSegments;
        float a0 = leftAngle + t0 * (rightAngle - leftAngle);
        float a1 = leftAngle + t1 * (rightAngle - leftAngle);

        Vector2 p0 = { screenOrigin.x + cosf(a0) * frustum.range,
                       screenOrigin.y + sinf(a0) * frustum.range };
        Vector2 p1 = { screenOrigin.x + cosf(a1) * frustum.range,
                       screenOrigin.y + sinf(a1) * frustum.range };
        DrawLineV(p0, p1, YELLOW);
    }
}

Texture2D makeGrayscaleTexture(const std::string& path) {
    Image img = LoadImage(path.c_str());
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // ensure we have alpha

    Color* pixels = LoadImageColors(img);
    for (int i = 0; i < img.width * img.height; i++) {
        unsigned char gray = (unsigned char)(
            pixels[i].r * 0.299f +
            pixels[i].g * 0.587f +
            pixels[i].b * 0.114f
        );
        pixels[i].r = gray;
        pixels[i].g = gray;
        pixels[i].b = gray;
        // pixels[i].a is left untouched
    }

    Image grayImg = {
        .data    = pixels,
        .width   = img.width,
        .height  = img.height,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D tex = LoadTextureFromImage(grayImg);
    UnloadImageColors(pixels);
    UnloadImage(img);
    return tex;
}

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
        const auto world = createWorld();

        // Grayscale texture cache for out-of-frustum objects
        std::unordered_map<std::string, Texture2D> grayTextures;

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            updatePlayer(player, dt);

            Frustum frustum;
            frustum.origin = {
                player.position.x + 16.0f,
                player.position.y + 16.0f
            };
            frustum.direction = getPlayerFacingDirection(player);
            frustum.halfAngle = 35.0f * DEG2RAD;
            frustum.range     = 200.0f;

            // Build the set of paths currently outside the frustum
            std::unordered_map<std::string, bool> outsideThisFrame;
            for (const auto& [position, assetPath] : world) {
                const std::string fullPath = assetsAbsolutePath + assetPath;
                if (!isInFrustum(frustum, position))
                    outsideThisFrame[fullPath] = true;
            }

            // Unload gray textures that are now inside the frustum
            for (auto it = grayTextures.begin(); it != grayTextures.end(); ) {
                if (outsideThisFrame.find(it->first) == outsideThisFrame.end()) {
                    UnloadTexture(it->second);
                    it = grayTextures.erase(it);
                } else {
                    ++it;
                }
            }

            // Load gray textures for newly outside objects
            for (const auto& [path, _] : outsideThisFrame) {
                if (grayTextures.find(path) == grayTextures.end())
                    grayTextures[path] = makeGrayscaleTexture(path);
            }

            BeginDrawing();
            ClearBackground(LIME);

            for (const auto& [position, assetPath] : world) {
                const std::string fullPath = assetsAbsolutePath + assetPath;
                const Vector2 screenPos = {
                    position.x + WORLD_ORIGIN.x,
                    position.y + WORLD_ORIGIN.y
                };

                if (isInFrustum(frustum, position)) {
                    const Texture2D tex = textureCache.get(fullPath);
                    DrawTexture(tex, screenPos.x, screenPos.y, WHITE);
                } else {
                    DrawTexture(grayTextures[fullPath], screenPos.x, screenPos.y, WHITE);
                }
            }

            drawFrustum(frustum, WORLD_ORIGIN);
            drawPlayer(player, textureCache, WORLD_ORIGIN);

            EndDrawing();
        }

        // Cleanup
        for (auto& [path, tex] : grayTextures)
            UnloadTexture(tex);

    } catch (const CacheError& e) {
        TraceLog(LOG_ERROR, "CacheError: %s", e.what());
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(e.what(), 10, 200, 18, RED);
            EndDrawing();
        }
    }

    CloseWindow();
}