#include "AssetEntry.h"
#include "AssetRef.h"

#include <iostream>
#include <ranges>

AssetEntry::AssetEntry(const fs::path &path, std::unique_ptr<std::byte[]> memPtr,
                       const uint32_t assetSize, std::list<fs::path>* registryLruList)
    : path(path), memPtr(std::move(memPtr)), assetSize(assetSize), numRefsLifetime(0), registryLruList(registryLruList) {
}

AssetEntry::~AssetEntry() {
    //std::cout << "AssetEntry destroyed: " << path << std::endl;
}

std::shared_ptr<AssetRef> AssetEntry::createRef() {
    numRefsLifetime++;
    // using numRefsLifetime to create a unique id for this ref
    auto ref = std::shared_ptr<AssetRef>(new AssetRef(shared_from_this(), numRefsLifetime));

    refs.insert({ref->id, ref});

    return ref;
}

void AssetEntry::freeRef(const AssetRef& ref) {
    refs.erase(ref.id);
    //lastRefFreedAt = std::chrono::system_clock::now();

    if (refs.empty() && registryLruList) {
        // moves this asset to the front of the LRU list for most-recently freed.
        // remember that only assets with zero refs active are in the LRU list
        registryLruList->emplace_front(path);
        lruIterator = registryLruList->begin();

        // note: registryLruList would be nullptr for AssetEntry unit tests
    }
}

std::span<const std::byte> AssetEntry::data() const {
    return std::span(memPtr.get(), assetSize);
}

size_t AssetEntry::getRefCount() const {
    return refs.size();
}

uint32_t AssetEntry::getAssetSize() const {
    return assetSize;
}

void AssetEntry::invalidateRefs() {
    for (auto& weakRef: refs | std::views::values) {
        if (const auto ref = weakRef.lock()) {
            ref->invalidate();
        }
    }
}
