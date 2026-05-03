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

void drawFrustumFilled(const Frustum& frustum, Vector2 worldOrigin, Color fillColor, Color outlineColor) {
    Vector2 screenOrigin = {
        frustum.origin.x + worldOrigin.x,
        frustum.origin.y + worldOrigin.y
    };
    float baseAngle  = atan2f(frustum.direction.y, frustum.direction.x);
    float leftAngle  = baseAngle - frustum.halfAngle;
    float rightAngle = baseAngle + frustum.halfAngle;

    int arcSegments = 40;
    for (int i = 0; i < arcSegments; i++) {
        float t0 = (float)i       / arcSegments;
        float t1 = (float)(i + 1) / arcSegments;
        float a0 = leftAngle + t0 * (rightAngle - leftAngle);
        float a1 = leftAngle + t1 * (rightAngle - leftAngle);

        Vector2 p0 = { screenOrigin.x + cosf(a0) * frustum.range,
                       screenOrigin.y + sinf(a0) * frustum.range };
        Vector2 p1 = { screenOrigin.x + cosf(a1) * frustum.range,
                       screenOrigin.y + sinf(a1) * frustum.range };

        DrawTriangle(screenOrigin, p0, p1, fillColor);
        if (outlineColor.a > 0) DrawLineV(p0, p1, outlineColor);
    }

    Vector2 leftEdge  = { screenOrigin.x + cosf(leftAngle)  * frustum.range,
                          screenOrigin.y + sinf(leftAngle)  * frustum.range };
    Vector2 rightEdge = { screenOrigin.x + cosf(rightAngle) * frustum.range,
                          screenOrigin.y + sinf(rightAngle) * frustum.range };
    if (outlineColor.a > 0) {
        DrawLineV(screenOrigin, leftEdge,  outlineColor);
        DrawLineV(screenOrigin, rightEdge, outlineColor);
    }
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
        std::unordered_map<std::string, Texture2D> grayTextures;

        // Load shader — null vertex shader means use raylib's default
        Shader frustumShader = LoadShader(nullptr,
            "C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/shaders/frustum.fsh");

        // Get uniform locations
        int originLoc    = GetShaderLocation(frustumShader, "frustumOrigin");
        int dirLoc       = GetShaderLocation(frustumShader, "frustumDirection");
        int angleLoc     = GetShaderLocation(frustumShader, "frustumHalfAngle");
        int rangeLoc     = GetShaderLocation(frustumShader, "frustumRange");
        int resLoc       = GetShaderLocation(frustumShader, "resolution");

        float resolution[2] = { 768.0f, 432.0f };
        SetShaderValue(frustumShader, resLoc, resolution, SHADER_UNIFORM_VEC2);

        // Render texture to draw the whole scene into before applying shader
        RenderTexture2D renderTarget = LoadRenderTexture(768, 432);

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

            // Update gray texture cache
            std::unordered_map<std::string, bool> outsideThisFrame;
            for (const auto& [position, assetPath] : world) {
                const std::string fullPath = assetsAbsolutePath + assetPath;
                if (!isInFrustum(frustum, position))
                    outsideThisFrame[fullPath] = true;
            }
            for (auto it = grayTextures.begin(); it != grayTextures.end(); ) {
                if (outsideThisFrame.find(it->first) == outsideThisFrame.end()) {
                    UnloadTexture(it->second);
                    it = grayTextures.erase(it);
                } else { ++it; }
            }
            for (const auto& [path, _] : outsideThisFrame) {
                if (grayTextures.find(path) == grayTextures.end())
                    grayTextures[path] = makeGrayscaleTexture(path);
            }

            // Update shader uniforms with current frustum state
            // Frustum origin is in screen space
            Vector2 screenOrigin = {
                frustum.origin.x + WORLD_ORIGIN.x,
                frustum.origin.y + WORLD_ORIGIN.y
            };
            float origin[2] = { screenOrigin.x, 432.0f - screenOrigin.y }; // Y flipped for render texture
            float dir[2]    = { frustum.direction.x, -frustum.direction.y }; // Y flipped for render texture
            float angle     = frustum.halfAngle;
            float range     = frustum.range;

            SetShaderValue(frustumShader, originLoc, origin, SHADER_UNIFORM_VEC2);
            SetShaderValue(frustumShader, dirLoc,    dir,    SHADER_UNIFORM_VEC2);
            SetShaderValue(frustumShader, angleLoc,  &angle, SHADER_UNIFORM_FLOAT);
            SetShaderValue(frustumShader, rangeLoc,  &range, SHADER_UNIFORM_FLOAT);

            // --- Draw scene into render texture ---
            BeginTextureMode(renderTarget);
                ClearBackground(LIME);
                for (const auto& [position, assetPath] : world) {
                    const std::string fullPath = assetsAbsolutePath + assetPath;
                    const Vector2 screenPos = {
                        position.x + WORLD_ORIGIN.x,
                        position.y + WORLD_ORIGIN.y
                    };
                    // Always draw full color into render texture — shader handles desaturation
                    const Texture2D tex = textureCache.get(fullPath);
                    DrawTexture(tex, screenPos.x, screenPos.y, WHITE);
                }
                drawPlayer(player, textureCache, WORLD_ORIGIN);
            EndTextureMode();

            // --- Composite render texture to screen through shader ---
            BeginDrawing();
                ClearBackground(BLACK);
                BeginShaderMode(frustumShader);
                    // RenderTexture is flipped vertically in raylib
                    DrawTextureRec(renderTarget.texture,
                        { 0, 0, 768, -432 }, // negative height flips it
                        { 0, 0 },
                        WHITE);
                EndShaderMode();

                // Draw frustum outline on top (no shader)
                drawFrustumFilled(frustum, WORLD_ORIGIN, { 0,0,0,0 }, BLACK);
            EndDrawing();
        }

        // Cleanup
        for (auto& [path, tex] : grayTextures) UnloadTexture(tex);
        UnloadShader(frustumShader);
        UnloadRenderTexture(renderTarget);

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