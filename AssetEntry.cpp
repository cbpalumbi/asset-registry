#include "AssetEntry.h"
#include "AssetRef.h"

#include <iostream>

AssetEntry::AssetEntry(std::unique_ptr<std::byte[]> memPtr, const uint32_t assetSize)
    : memPtr(std::move(memPtr)), assetSize(assetSize), numRefsLifetime(0) {
}

std::shared_ptr<AssetRef> AssetEntry::createRef() {
    numRefsLifetime++;
    return std::make_shared<AssetRef>(shared_from_this(), numRefsLifetime);
}

void AssetEntry::freeRef(uint32_t refId) {
    refs.erase(refId);
    lastRefFreedAt = std::chrono::system_clock::now();
}

std::optional<std::chrono::duration<double> > AssetEntry::getTimeSinceLastRefFreed() const {

    if (lastRefFreedAt) {
        return std::chrono::system_clock::now() - lastRefFreedAt.value();
    }

    std::cout << "No refs for this asset have ever been freed." << "\n";
    return std::nullopt;
}
