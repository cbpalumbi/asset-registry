#include "Registry.h"
#include "CacheError.h"

#include <iostream>
#include <fstream>
#include <ranges>

bool isValidPath(const fs::path& p) {
    return fs::exists(p) && !fs::is_directory(p);
}

std::optional<std::shared_ptr<AssetRef>> Registry::load(fs::path const &path) {

    if (!isValidPath(path)) {
        std::cout << "Path " << path << "is not valid." << "\n";
        throw AssetNotFoundError(path);
    }

    // Check if the asset is in the cache already.
    // the asset is in the cache already if its path exists in the entries map
    if (entries.contains(path)) {
        // return a new asset ref for it
        auto ref = entries[path]->createRef();

        // if it's in the lruList (aka its lruIterator is not null), remove it.
        // this would be the case if there were no active refs for it,
        // but it hadn't been evicted yet
        if (entries[path]->lruIterator) {
            lruList.erase(*entries[path]->lruIterator);
        }
        return ref;
    }

    const auto fileSize = std::filesystem::file_size(path);
    if (fileSize > CACHE_CAPACITY) {
        throw AssetSizeExceedsCacheCapacityError(path);
    }

    // otherwise, attempt to load into cache normally
    if (canFitInCacheWithoutEviction(fileSize)) {
        auto assetRef = loadIntoCache(path);
        if (!assetRef) {
            throw CacheError("An error occurred when loading the asset into the cache.");
        }
        return assetRef;
    }

    // if it can't fit, see if the size of the evictable assets would be enough to make room for the new one
    const auto evictableList = tryGetEvictableList();

    // TODO: potential optimization to minimize list traversal - combine size check?
    if (evictableList.empty() || !canFitInCacheWithEviction(fileSize)) {
        throw NoSpaceInCacheError(path);
    }

    uintmax_t emptySpaceInCache = CACHE_CAPACITY - getCurrentUsage();
    for (const auto& item : evictableList) {
        // evict the item
        emptySpaceInCache += evictAssetByPath(item);
        // pop from the back of the lruList
        lruList.pop_back();

        // break out of the loop once we've evicted enough to fit the new item
        if (emptySpaceInCache >= fileSize) break;
    }

    return loadIntoCache(path);
}


std::optional<std::shared_ptr<AssetRef>> Registry::loadIntoCache(fs::path const &path) {

    // determine the size of the file
    auto fileSize = std::filesystem::file_size(path);

    // open the file with a flag specifying mode as binary
    std::ifstream stream;
    stream.open(path, std::ios::binary);
    if (stream.fail()) {
        throw CacheError("Error reading file: " + path.string());
    }

    auto memPtr = std::make_unique<std::byte[]>(fileSize);

    // stream.read expects a char* to destination location. need to cast
    stream.read(reinterpret_cast<char*>(memPtr.get()), fileSize);
    stream.close();

    // create a new AssetEntry
    auto entry = std::make_shared<AssetEntry>(path, std::move(memPtr), fileSize, &lruList);
    entries.insert({path, entry});

    std::shared_ptr<AssetRef> ref = entry->createRef();

    return ref;
}

bool Registry::canFitInCacheWithoutEviction(const uintmax_t fileSize) const {
    return CACHE_CAPACITY - getCurrentUsage() >= fileSize;
}

bool Registry::canFitInCacheWithEviction(const uintmax_t fileSize) const {
    const uint32_t evictableBytes = getSizeOfEvictableBytes();
    const uint32_t nonEvictableBytes = getCurrentUsage() - evictableBytes;
    return CACHE_CAPACITY - nonEvictableBytes >= fileSize;
}

std::list<fs::path> Registry::tryGetEvictableList() {
    return lruList;
}

uint32_t Registry::getCurrentUsage() const {
    uint32_t sum = 0;
    for (const auto &entryPtr: entries | std::views::values) {
        sum += entryPtr->getAssetSize();
    }

    return sum;
}

uintmax_t Registry::getSizeOfEvictableBytes() const {
    uintmax_t sum = 0;
    for (auto p : lruList) {
        sum += entries.at(p)->assetSize;
    }
    return sum;
}

std::optional<std::shared_ptr<AssetEntry>> Registry::getEntryByPath(fs::path const &path) {
    if (entries.contains(path)) {
        return entries[path];
    }
    return std::nullopt;
}

uintmax_t Registry::evictAssetByPath(fs::path const &path) {
    if (entries.contains(path)) {
        const auto entry = entries[path];
        return evictAsset(entry);
    }
    return 0;
}

uintmax_t Registry::evictAsset(std::shared_ptr<AssetEntry> entry) {
    // for the evicted assets, follow the AssetEntry to their AssetRefs and invalidate them.
    entry->invalidateRefs();

    // free the memory - std::unique_ptr gets freed when goes out of scope
    entry->memPtr = nullptr;

    const auto tmpSize = entry->getAssetSize();

    // destroy the AssetEntry
    entries.erase(entry->path);
    entry.reset();

    return tmpSize;
}

bool Registry::hasEntryInCache(fs::path const &path) const {
    return entries.contains(path);
}

