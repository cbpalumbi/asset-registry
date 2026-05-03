#include "Player.h"

std::string getAnimationFolder(const Player& player) {
    if (!player.moving) return player.lastMovingFolder;
    switch (player.direction) {
        case PlayerDirection::Down:     return "Rogue Animationset/DownWalk";
        case PlayerDirection::DownSide: return "Rogue Animationset/DownSideWalk";
        case PlayerDirection::Side:     return "Rogue Animationset/SideWalk";
        case PlayerDirection::UpSide:   return "Rogue Animationset/UpSideWalk";
        case PlayerDirection::Up:       return "Rogue Animationset/UpWalk";
        default:                        return "Rogue Animationset/Idle";
    }
}

std::string getFramePath(const Player& player) {
    const std::string assetsAbsolutePath = "C:/Users/Bella/CLionProjects/AssetRegistry/game/assets/";
    const std::string folder = assetsAbsolutePath + getAnimationFolder(player);
    const std::string frameName = folder.substr(folder.find_last_of('/') + 1);
    return folder + "/" + frameName + std::to_string(player.currentFrame + 1) + ".png";
}

void updatePlayer(Player& player, float dt) {
    Vector2 movement = { 0, 0 };

    const bool up    = IsKeyDown(KEY_W);
    const bool down  = IsKeyDown(KEY_S);
    const bool left  = IsKeyDown(KEY_A);
    const bool right = IsKeyDown(KEY_D);

    if (up && right)        { movement = { 1, -1 };  player.direction = PlayerDirection::UpSide;   player.facingLeft = false; }
    else if (up && left)    { movement = { -1, -1 }; player.direction = PlayerDirection::UpSide;   player.facingLeft = true;  }
    else if (down && right) { movement = { 1, 1 };   player.direction = PlayerDirection::DownSide; player.facingLeft = true;  }
    else if (down && left)  { movement = { -1, 1 };  player.direction = PlayerDirection::DownSide; player.facingLeft = false; }
    else if (up)            { movement = { 0, -1 };  player.direction = PlayerDirection::Up;        }
    else if (down)          { movement = { 0, 1 };   player.direction = PlayerDirection::Down;      }
    else if (right)         { movement = { 1, 0 };   player.direction = PlayerDirection::Side;      player.facingLeft = true;  }
    else if (left)          { movement = { -1, 0 };  player.direction = PlayerDirection::Side;      player.facingLeft = false; }

    player.moving = (movement.x != 0 || movement.y != 0);

    if (player.moving) {
        switch (player.direction) {
            case PlayerDirection::Down:     player.lastMovingFolder = "Rogue Animationset/DownWalk"; break;
            case PlayerDirection::DownSide: player.lastMovingFolder = "Rogue Animationset/DownSideWalk"; break;
            case PlayerDirection::Side:     player.lastMovingFolder = "Rogue Animationset/SideWalk"; break;
            case PlayerDirection::UpSide:   player.lastMovingFolder = "Rogue Animationset/UpSideWalk"; break;
            case PlayerDirection::Up:       player.lastMovingFolder = "Rogue Animationset/UpWalk"; break;
        }
    }

    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.707f;
        movement.y *= 0.707f;
    }

    player.position.x += movement.x * player.SPEED * dt;
    player.position.y += movement.y * player.SPEED * dt;

    std::string newFolder = getAnimationFolder(player);
    if (newFolder != player.currentFolder) {
        player.currentFolder = newFolder;
        player.currentFrame = 0;
        player.frameTimer = 0;
    }

    player.frameTimer += dt;
    if (player.frameTimer >= player.FRAME_DURATION) {
        player.frameTimer = 0;
        player.currentFrame = (player.currentFrame + 1) % 6;
    }
}

void drawPlayer(Player& player, Vector2 worldOrigin) {
    std::string path = getFramePath(player);

    if (path != player.currentTexturePath) {
        if (!player.currentTexturePath.empty())
            UnloadTexture(player.currentTexture);
        player.currentTexture = LoadTexture(path.c_str());
        player.currentTexturePath = path;
    }

    if (player.facingLeft) {
        DrawTextureRec(player.currentTexture,
            { 0, 0, -32, 32 },
            { player.position.x + worldOrigin.x, player.position.y + worldOrigin.y },
            WHITE);
    } else {
        DrawTexture(player.currentTexture,
            player.position.x + worldOrigin.x,
            player.position.y + worldOrigin.y,
            WHITE);
    }
}

void unloadPlayer(Player& player) {
    if (!player.currentTexturePath.empty())
        UnloadTexture(player.currentTexture);
}