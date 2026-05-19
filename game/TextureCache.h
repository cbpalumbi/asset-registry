#pragma once
#include "raylib.h"
#include "Registry.h"
#include "CacheError.h"
#include "AssetRef.h"

#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <ranges>

namespace fs = std::filesystem;

// Go-between for raylib and AssetRegistry
#pragma once
#include "raylib.h"
#include "Registry.h"
#include "CacheError.h"
#include "AssetRef.h"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <ranges>

namespace fs = std::filesystem;

class TextureCache {
    Registry& registry;
    std::unordered_map<fs::path, Texture2D> gpuTextures;
    std::unordered_map<fs::path, std::shared_ptr<AssetRef>> assetRefs;
    std::unordered_set<fs::path> trackedPaths; // what was in frustum last frame

public:
    explicit TextureCache(Registry& registry) : registry(registry) {}

    // Call once per frame with the canonical paths currently in the frustum.
    // Automatically loads new entries and evicts ones that left.
    void update(const std::unordered_set<fs::path>& inFrustumThisFrame) {
        // Evict anything that dropped out of the frustum
        for (const auto& path : trackedPaths) {
            if (!inFrustumThisFrame.contains(path))
                evict(path);
        }
        // Load anything newly inside the frustum
        for (const auto& path : inFrustumThisFrame) {
            if (!trackedPaths.contains(path))
                load(path);
        }
        trackedPaths = inFrustumThisFrame;
    }

    // Safe to call even if already loaded — returns cached texture
    Texture2D get(const fs::path& path) const {
        if (gpuTextures.contains(path))
            return gpuTextures.at(path);
        throw CacheError("get() called on path not in GPU cache: " + path.string());
    }

    ~TextureCache() {
        for (const auto& tex : gpuTextures | std::views::values)
            UnloadTexture(tex);
    }

private:
    void load(const fs::path& path) {
        if (gpuTextures.contains(path)) return;

        const auto result = registry.load(path);
        if (!result.has_value())
            throw CacheError("Failed to load asset: " + path.string());

        assetRefs[path] = result.value();

        const auto bytes = result.value()->data();
        Image img = LoadImageFromMemory(".png",
            reinterpret_cast<const unsigned char*>(bytes.data()),
            static_cast<int>(bytes.size()));
        gpuTextures[path] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    void evict(const fs::path& path) {
        if (gpuTextures.contains(path)) {
            UnloadTexture(gpuTextures[path]);
            gpuTextures.erase(path);
        }
        assetRefs.erase(path); // shared_ptr drops, registry can evict
    }
};