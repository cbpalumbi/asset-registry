#include "Player.h"

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

