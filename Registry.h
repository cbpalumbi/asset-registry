#ifndef ASSETREGISTRY_REGISTRY_H
#define ASSETREGISTRY_REGISTRY_H

#include "AssetEntry.h"
#include <cstdint>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

class Registry {
friend class RegistryTest;
public:
    Registry();

    std::optional<std::shared_ptr<AssetRef>> Load(fs::path const &path);
    uint32_t GetCurrentUsage();
    const uint32_t CACHE_CAPACITY = 2048; // 2 KB for now
private:

    std::unordered_map<fs::path, std::shared_ptr<AssetEntry>> entries;

    std::optional<std::shared_ptr<AssetRef>> LoadIntoCache(fs::path const &path);

    bool CanFitInCache(uintmax_t fileSize);
};


#endif //ASSETREGISTRY_REGISTRY_H