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
};

std::string getAnimationFolder(const Player& player);
std::string getFramePath(const Player& player);
void updatePlayer(Player& player, float dt);
void drawPlayer(Player& player, TextureCache& textureCache, Vector2 worldOrigin);


#endif //ASSETREGISTRY_PLAYER_H