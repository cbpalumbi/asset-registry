#include "AssetEntry.h"
#include "AssetRef.h"

#include <iostream>

AssetEntry::AssetEntry(std::unique_ptr<std::byte[]> memPtr, const uint32_t assetSize)
    : memPtr(std::move(memPtr)), assetSize(assetSize), numRefsLifetime(0) {
}

std::shared_ptr<AssetRef> AssetEntry::createRef() {
    numRefsLifetime++;
    // using numRefsLiftime to create a unique id for this ref
    auto ref = std::make_shared<AssetRef>(shared_from_this(), numRefsLifetime);

    refs.insert({ref->id, ref});

    return ref;
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

std::span<const std::byte> AssetEntry::data() const {
    return std::span(memPtr.get(), assetSize);
}
