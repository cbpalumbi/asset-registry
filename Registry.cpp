#include "Registry.h"

#include <iostream>
#include <fstream>
#include <ranges>


Registry::Registry() {
    std::cout << "Registry created\n";
    std::cout << "Num entries: " << entries.size() << "\n";
}

bool isValidPath(const fs::path& p) {
    return fs::exists(p) && !fs::is_directory(p);
}

std::optional<std::shared_ptr<AssetRef>> Registry::load(fs::path const &path) {

    if (!isValidPath(path)) {
        std::cout << "Path " << path << "is not valid." << "\n";
        return std::nullopt;
    }

    // Check if the asset is in the cache already.
    // the asset is in the cache already if its path exists in the entries map
    if (entries.contains(path)) {
        // return a new asset ref for it
        auto ref = entries[path]->createRef();

        // if it's in the lruList (lruIterator not null), remove it.
        // this would be the case if there were no active refs for it,
        // but it hadn't been evicted yet
        if (entries[path]->lruIterator) {
            lruList.erase(*entries[path]->lruIterator);
        }
        return ref;
    }

    const auto fileSize = std::filesystem::file_size(path);
    if (fileSize > CACHE_CAPACITY) {
        return std::nullopt; // TODO: Add CacheError to returned
    }

    // otherwise, attempt to load into cache
    if (canFitInCache(fileSize)) {
        auto assetRef = loadIntoCache(path);
        if (!assetRef) {
            return std::nullopt;
        }
        return assetRef;
    } else {
        auto evictList = tryGetEvictList(fileSize);
        if (!evictList) {
            return std::nullopt; // TODO: Add CacheError returned
        }

        for (const auto& item : *evictList) {
            // evict the item
            evictAsset(item);
        }

        // if yes, great

            // load the new asset into memory. create  the AssetEntry for it, including creating a new AssetRef and
            // return true, AssetRef*.
        // if not, return false
    }

    return std::nullopt;
}


std::optional<std::shared_ptr<AssetRef>> Registry::loadIntoCache(fs::path const &path) {

    // determine the size of the file
    auto fileSize = std::filesystem::file_size(path);
    std::cout << "Filesize is " << fileSize << "\n";

    // open the file with a flag specifying mode as binary
    std::ifstream stream;
    stream.open(path, std::ios::binary);
    if (stream.fail()) {
        std::cout << "Error opening file " << path << "\n";
        return std::nullopt;
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

bool Registry::canFitInCache(const uintmax_t fileSize) {
    const uint32_t currentUsage = getCurrentUsage();
    return CACHE_CAPACITY - currentUsage >= fileSize;
}

std::optional<std::vector<std::shared_ptr<AssetEntry>>> Registry::tryGetEvictList(const uintmax_t fileSize) {
    // loop through the asset entries and find ones with zero refs

    uintmax_t sumEvictableBytes = 0;
    std::vector<std::shared_ptr<AssetEntry>> evictableEntries;

    if (entries.empty()) {
        return evictableEntries;
    }

    for (const std::shared_ptr<AssetEntry> &val: entries | std::views::values) {
        if (val->getRefCount() == 0) {
            sumEvictableBytes += val->getAssetSize();
            evictableEntries.push_back(val);
            if (sumEvictableBytes >= fileSize) {
                return evictableEntries;
            }
        }
    }
    return std::nullopt;
}

uint32_t Registry::getCurrentUsage() {
    uint32_t sum = 0;
    for (const auto &val: entries | std::views::values) {
        sum += val->getAssetSize();
    }

    return sum;
}


bool Registry::evictAssetByPath(fs::path const &path) {
    if (entries.contains(path)) {
        const auto entry = entries[path];
        return evictAsset(entry);
    }
    return false;
}

bool Registry::evictAsset(std::shared_ptr<AssetEntry> entry) {
    // for the evicted assets, follow the AssetEntry to their AssetRefs and invalidate them.
    entry->invalidateRefs();

    // free the memory - std::unique_ptr gets freed when goes out of scope
    entry->memPtr = nullptr;

    // destroy the AssetEntry
    entries.erase(entry->path);
    entry.reset();
    return true;
}

bool Registry::hasEntryInCache(fs::path const &path) const {
    return entries.contains(path);
}

