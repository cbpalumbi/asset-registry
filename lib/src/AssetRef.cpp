#include "AssetRef.h"
#include "AssetEntry.h"


void AssetRef::invalidate() {
    assetEntry = nullptr;
}

std::span<const std::byte> AssetRef::data() const {
    if (!assetEntry) throw std::runtime_error("AssetRef has been invalidated — asset was evicted");
    return assetEntry->data();
}
