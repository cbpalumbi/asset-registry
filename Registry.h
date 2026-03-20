#ifndef ASSETREGISTRY_REGISTRY_H
#define ASSETREGISTRY_REGISTRY_H

#include "AssetEntry.h"
#include <cstdint>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Registry {
public:
    Registry();

    bool Load(fs::path const &path);
    uint32_t GetCurrentUsage();
private:

    const uint32_t cacheCapacity = 2048; // 2 KB for now
    std::vector<std::shared_ptr<AssetEntry>> entries;


};


#endif //ASSETREGISTRY_REGISTRY_H