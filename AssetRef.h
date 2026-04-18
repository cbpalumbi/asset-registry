#ifndef ASSETREGISTRY_ASSETREF_H
#define ASSETREGISTRY_ASSETREF_H

#include <memory>
#include <span>

#include "AssetEntry.h"

class AssetRefTests;

class AssetRef {
    friend class AssetEntry;
    friend class AssetRefTests;

    std::shared_ptr<AssetEntry> assetEntry;
    uint32_t id;

    // RAII constructor
    AssetRef(std::shared_ptr<AssetEntry> assetEntry, const uint32_t id) : assetEntry(assetEntry), id(id) {};
    void invalidate();

public:
    std::span<const std::byte> data() const;
    // RAII destructor, public so shared_ptr can use
    ~AssetRef() { if (assetEntry) assetEntry->freeRef(*this); }
};


#endif //ASSETREGISTRY_ASSETREF_H