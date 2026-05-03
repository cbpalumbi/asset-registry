#ifndef ASSETREGISTRY_PLAYER_H
#define ASSETREGISTRY_PLAYER_H

#include "TextureCache.h"
#include <string>
#include "raylib.h"

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
    std::string lastMovingFolder = "Rogue Animationset/Idle";
    Texture2D currentTexture = {};
    std::string currentTexturePath;
};

void drawPlayer(Player& player, Vector2 worldOrigin);
void unloadPlayer(Player& player);
std::string getAnimationFolder(const Player& player);
std::string getFramePath(const Player& player);
void updatePlayer(Player& player, float dt);



#endif //ASSETREGISTRY_PLAYER_H