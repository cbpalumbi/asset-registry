#ifndef ASSETREGISTRY_REGISTRY_H
#define ASSETREGISTRY_REGISTRY_H

#include "AssetEntry.h"
#include <cstdint>
#include <iostream>
#include <vector>



class Registry {
public:
    Registry();
private:

    const uint32_t cacheCapacity = 2048; // 2 KB for now
    std::vector<AssetEntry> entries;

};


#endif //ASSETREGISTRY_REGISTRY_H