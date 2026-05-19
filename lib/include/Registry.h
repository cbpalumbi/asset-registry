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
    std::optional<std::shared_ptr<AssetRef>> load(fs::path const &path);
    [[nodiscard]] uint32_t getCurrentUsage() const;
    [[nodiscard]] std::vector<std::string> getCurrentEntryNames() const;
    const uint32_t CACHE_CAPACITY = 8000;
private:

    std::unordered_map<fs::path, std::shared_ptr<AssetEntry>> entries;

    // Representation of an LRU list but here LRU is equivalent to "Least Recently Freed, But Still In Cache"
    // or "Oldest Freed Ref Still in the Cache".
    // Also represents currently evictable entries.
    // IMPORTANT: BACK of the list represents the next asset to evict.
    std::list<fs::path> lruList;

    std::optional<std::shared_ptr<AssetRef>> loadIntoCache(fs::path const &path, uintmax_t fileSize);
    [[nodiscard]] bool canFitInCacheWithoutEviction(uintmax_t fileSize) const;
    [[nodiscard]] bool canFitInCacheWithEviction(uintmax_t fileSize) const;
    std::list<fs::path> tryGetEvictableList();
    // returns size of evicted asset in bytes
    uintmax_t evictAssetByPath(fs::path const &path);
    uintmax_t evictAsset(std::shared_ptr<AssetEntry> entry);
    [[nodiscard]] bool hasEntryInCache(fs::path const &path) const;
    [[nodiscard]] uintmax_t getSizeOfEvictableBytes() const;
    std::optional<std::shared_ptr<AssetEntry>> getEntryByPath(fs::path const &path);
};


#endif //ASSETREGISTRY_REGISTRY_H
