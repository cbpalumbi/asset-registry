#ifndef ASSETREGISTRY_ASSETENTRY_H
#define ASSETREGISTRY_ASSETENTRY_H
#include <cstdint>


class AssetEntry {
    const uint8_t myNum;
    AssetEntry(const uint8_t t) : myNum(t) {}

public:
    uint8_t get() const { return myNum; }

};


#endif //ASSETREGISTRY_ASSETENTRY_H