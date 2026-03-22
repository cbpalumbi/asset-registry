#ifndef ASSETREGISTRY_ASSETENTRY_H
#define ASSETREGISTRY_ASSETENTRY_H

#include <cstdint>
#include <memory>
#include <chrono>
#include <unordered_map>

using timestamp = std::chrono::time_point<std::chrono::system_clock>;
class AssetRef;

class AssetEntry : public std::enable_shared_from_this<AssetEntry> {
    std::unordered_map<uint32_t, std::weak_ptr<AssetRef>> refs;
    std::optional<timestamp> lastRefFreedAt;
    std::unique_ptr<std::byte[]> memPtr;
    uint32_t assetSize;
    int16_t numRefsLifetime;

public:
    AssetEntry(std::unique_ptr<std::byte[]> memPtr, uint32_t assetSize);
    std::shared_ptr<AssetRef> createRef();
    void freeRef(uint32_t refId);
    std::optional<std::chrono::duration<double>> getTimeSinceLastRefFreed() const;

};


#endif //ASSETREGISTRY_ASSETENTRY_H