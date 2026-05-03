#pragma once
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

class CacheError : public std::runtime_error {
public:
    explicit CacheError(const std::string& message)
        : std::runtime_error(message) {}
};

class AssetNotFoundError : public CacheError {
public:
    explicit AssetNotFoundError(const fs::path& path)
        : CacheError("Asset not found: " + path.string()) {}
};

class AssetSizeExceedsCacheCapacityError : public CacheError {
public:
    explicit AssetSizeExceedsCacheCapacityError(const fs::path& path)
        : CacheError("Asset " + path.string() + " is larger than the capacity of the cache.") {}
};

class NoSpaceInCacheError : public CacheError {
public:
    explicit NoSpaceInCacheError(const fs::path& path, const uint32_t fileSize)
        : CacheError("There is no space in the cache for asset " + path.string() + " with size " + std::to_string(fileSize)) {}
};

class EvictionError : public CacheError {
public:
    explicit EvictionError(const fs::path& path)
        : CacheError("Error evicting asset: " + path.string()) {}
};