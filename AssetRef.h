#ifndef ASSETREGISTRY_ASSETREF_H
#define ASSETREGISTRY_ASSETREF_H

#include <memory>
#include <span>

class AssetRef {
    friend class AssetEntry;

    std::shared_ptr<AssetEntry> assetEntry;
    uint32_t id;

public:
    AssetRef(std::shared_ptr<AssetEntry> assetEntry, uint32_t id) : assetEntry(assetEntry), id(id) {};
    ~AssetRef();

    std::span<const std::byte> data() const;


};


#endif //ASSETREGISTRY_ASSETREF_H