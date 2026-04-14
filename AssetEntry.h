#ifndef ASSETREGISTRY_ASSETENTRY_H
#define ASSETREGISTRY_ASSETENTRY_H

#include <cstdint>
#include <memory>
#include <chrono>
#include <span>
#include <unordered_map>

using timestamp = std::chrono::time_point<std::chrono::system_clock>;
class AssetRef;

class AssetEntry : public std::enable_shared_from_this<AssetEntry> {
    friend class AssetRefTests;

    // {id, ref} - id is unique within this AssetEntry
    std::unordered_map<uint32_t, std::weak_ptr<AssetRef>> refs;
    std::optional<timestamp> lastRefFreedAt;
    std::unique_ptr<std::byte[]> memPtr;
    uint32_t assetSize;
    int16_t numRefsLifetime;

public:
    AssetEntry(std::unique_ptr<std::byte[]> memPtr, uint32_t assetSize);

    std::shared_ptr<AssetRef> createRef();
    void freeRef(const AssetRef& ref);
    std::optional<std::chrono::duration<double>> getTimeSinceLastRefFreed() const;
    std::span<const std::byte> data() const;
    size_t getRefCount() const;

};


#endif //ASSETREGISTRY_ASSETENTRY_H