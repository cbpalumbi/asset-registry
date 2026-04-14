#include "AssetRef.h"

#include "AssetEntry.h"


std::span<const std::byte> AssetRef::data() const {
    return assetEntry->data();
}
