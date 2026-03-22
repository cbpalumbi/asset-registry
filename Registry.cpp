#include "Registry.h"

#include <iostream>
#include <fstream>

Registry::Registry() {
    std::cout << "Registry created\n";
    std::cout << "Num entries: " << entries.size() << "\n";
}

bool Registry::Load(fs::path const &path) {

    // check if asset is in cache
    const bool assetIsInCache = false;

    if (!assetIsInCache) {
        LoadIntoCache(path);
    }

    // if not, check if it can fit in the cache
        // if there is space, great
        // load the item into the cache
    // if not, check if it's possible to evict enough assets so that there will be room for it
        // if yes, great
            // for the evicted assets, follow the AssetEntry to their AssetRefs and invalidate them. then copy the AssetEntry's mem_ptr into a temp var. then set mem_ptr to nullptr. then free the memory using the temp ptr.
            // load the new asset into memory. create or update the AssetEntry for it, including creating a new AssetRef and
            // return true, AssetRef*.
        // if not, return false

    return true;
}

uint32_t Registry::GetCurrentUsage() {
    return 4;
}

bool Registry::LoadIntoCache(fs::path const &path) {
    if (entries.contains(path)) {
        // update entry
        return false;
    }

    // load the bytes into memory with error handling

    // open the file

    std::ifstream stream;
    stream.setf(std::ios::binary);
    stream.open(path);
    if (stream.fail()) {
        std::cout << "Error reading file " << path << "\n";
        return false;
    }
    stream.close();


    // create a new AssetEntry



    entries.insert({path, std::make_shared<AssetEntry>()});
}
