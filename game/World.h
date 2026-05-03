#ifndef ASSETREGISTRY_WORLD_H
#define ASSETREGISTRY_WORLD_H

#include <string>
#include <vector>
#include "raylib.h"

struct WorldObject {
    Vector2 position;
    std::string assetPath;
};

std::vector<WorldObject> createWorld();

#endif //ASSETREGISTRY_WORLD_H