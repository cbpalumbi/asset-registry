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
    uint32_t getCurrentUsage() const;
    const uint32_t CACHE_CAPACITY = 2048; // 2 KB for now
private:

    std::unordered_map<fs::path, std::shared_ptr<AssetEntry>> entries;

    // Representation of an LRU list but here LRU is equivalent to "Least Recently Freed, But Still In Cache"
    // or "Oldest Freed Ref Still in the Cache".
    // Also represents currently evictable entries.
    // IMPORTANT: BACK of the list represents the next asset to evict.
    std::list<fs::path> lruList;

    std::optional<std::shared_ptr<AssetRef>> loadIntoCache(fs::path const &path);
    bool canFitInCacheWithoutEviction(uintmax_t fileSize) const;
    bool canFitInCacheWithEviction(uintmax_t fileSize) const;
    std::list<fs::path> tryGetEvictableList(uintmax_t fileSize);
    // returns size of evicted asset in bytes
    uintmax_t evictAssetByPath(fs::path const &path);
    uintmax_t evictAsset(std::shared_ptr<AssetEntry> entry);
    bool hasEntryInCache(fs::path const &path) const;
    uintmax_t getSizeOfEvictableBytes() const;
    std::optional<std::shared_ptr<AssetEntry>> getEntryByPath(fs::path const &path);
};


#endif //ASSETREGISTRY_REGISTRY_H
