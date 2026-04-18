#ifndef ASSETREGISTRY_REGISTRY_H
#define ASSETREGISTRY_REGISTRY_H

#include "AssetEntry.h"
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <list>

namespace fs = std::filesystem;

class Registry {
friend class RegistryTest;
public:
    Registry();

    std::optional<std::shared_ptr<AssetRef>> load(fs::path const &path);
    uint32_t getCurrentUsage();
    const uint32_t CACHE_CAPACITY = 2048; // 2 KB for now
private:

    std::unordered_map<fs::path, std::shared_ptr<AssetEntry>> entries;
    std::list<fs::path> lruList;

    std::optional<std::shared_ptr<AssetRef>> loadIntoCache(fs::path const &path);
    bool canFitInCache(uintmax_t fileSize);
    std::optional<std::vector<std::shared_ptr<AssetEntry>>> tryGetEvictList(uintmax_t fileSize);
    bool evictAssetByPath(fs::path const &path);
    bool evictAsset(std::shared_ptr<AssetEntry> entry);
    bool hasEntryInCache(fs::path const &path) const;
};


#endif //ASSETREGISTRY_REGISTRY_H
