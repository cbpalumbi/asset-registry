#ifndef ASSETREGISTRY_REGISTRY_H
#define ASSETREGISTRY_REGISTRY_H

#include "AssetEntry.h"
#include <cstdint>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

class Registry {
public:
    Registry();

    std::optional<std::shared_ptr<AssetRef>> Load(fs::path const &path);
    uint32_t GetCurrentUsage();
private:

    const uint32_t cacheCapacity = 2048; // 2 KB for now

    std::unordered_map<fs::path, std::shared_ptr<AssetEntry>> entries;

    std::optional<std::shared_ptr<AssetRef>> LoadIntoCache(fs::path const &path);
};


#endif //ASSETREGISTRY_REGISTRY_H