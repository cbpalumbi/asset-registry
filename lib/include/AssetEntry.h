#ifndef ASSETREGISTRY_ASSETENTRY_H
#define ASSETREGISTRY_ASSETENTRY_H

#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;
class AssetRef;

class AssetEntry : public std::enable_shared_from_this<AssetEntry> {
    friend class AssetRefTests;
    friend class Registry;

    fs::path path;
    std::unique_ptr<std::byte[]> memPtr;
    uint32_t assetSize;

    std::list<fs::path>* registryLruList;

    // {id, ref} - id is unique within this AssetEntry
    std::unordered_map<uint32_t, std::weak_ptr<AssetRef>> refs;

    int16_t numRefsLifetime;
    //bool isInCache;
    std::optional<std::list<fs::path>::iterator> lruIterator;

public:
    AssetEntry(const fs::path &path, std::unique_ptr<std::byte[]> memPtr, uint32_t assetSize, std::list<fs::path>* registryLruList);

    std::shared_ptr<AssetRef> createRef();
    void freeRef(const AssetRef& ref);
    std::span<const std::byte> data() const;
    size_t getRefCount() const;
    uint32_t getAssetSize() const;
    void invalidateRefs();

};


#endif //ASSETREGISTRY_ASSETENTRY_H