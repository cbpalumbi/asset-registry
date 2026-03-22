#include "AssetRef.h"

#include "AssetEntry.h"

AssetRef::~AssetRef() {

}

std::span<const std::byte> AssetRef::data() const {
    return assetEntry->data();
}
