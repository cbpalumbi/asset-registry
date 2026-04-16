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

std::optional<std::shared_ptr<AssetRef>> Registry::Load(fs::path const &path) {

    if (!isValidPath(path)) {
        std::cout << "Path " << path << "is not valid." << "\n";
        return std::nullopt;
    }

    // the asset is in the cache already if its path exists in the entries map
    if (entries.contains(path)) {
        // return a new asset ref for it
        return entries[path]->createRef();
    }

    const auto fileSize = std::filesystem::file_size(path);
    if (fileSize > CACHE_CAPACITY) {
        return std::nullopt; // TODO: Add CacheError to returned
    }

    // otherwise, attempt to load into cache
    if (CanFitInCache(fileSize)) {
        auto assetRef = LoadIntoCache(path);
        if (!assetRef) {
            return std::nullopt;
        }
        return assetRef;
    } else {
        auto evictList = TryGetEvictList(fileSize);
        if (!evictList) {
            return std::nullopt; // TODO: Add CacheError returned
        }
        // if yes, great
            // for the evicted assets, follow the AssetEntry to their AssetRefs and invalidate them.
            // then copy the AssetEntry's mem_ptr into a temp var.
            // then set mem_ptr to nullptr. then free the memory using the temp ptr.
            // load the new asset into memory. create or update the AssetEntry for it, including creating a new AssetRef and
            // return true, AssetRef*.
        // if not, return false
    }

    return std::nullopt;
}


std::optional<std::shared_ptr<AssetRef>> Registry::LoadIntoCache(fs::path const &path) {
    if (entries.contains(path)) {
        // TODO: update entry to indicate it's in the cache

        return std::nullopt;
    }

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
    auto entry = std::make_shared<AssetEntry>(std::move(memPtr), fileSize);
    entries.insert({path, entry});

    std::shared_ptr<AssetRef> ref = entry->createRef();

    return ref;
}

bool Registry::CanFitInCache(const uintmax_t fileSize) {
    const uint32_t currentUsage = GetCurrentUsage();
    return CACHE_CAPACITY - currentUsage >= fileSize;
}

std::optional<std::vector<std::shared_ptr<AssetEntry>>> Registry::TryGetEvictList(const uintmax_t fileSize) {
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

uint32_t Registry::GetCurrentUsage() {
    uint32_t sum = 0;
    for (const auto &val: entries | std::views::values) {
        sum += val->getAssetSize();
    }

    return sum;
}
