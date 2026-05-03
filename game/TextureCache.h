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
class TextureCache {
    Registry& registry;
    std::unordered_map<fs::path, Texture2D> gpuTextures;
    std::unordered_map<fs::path, std::shared_ptr<AssetRef>> assetRefs; // keep refs alive

public:
    explicit TextureCache(Registry& registry) : registry(registry) {}

    Texture2D get(const fs::path& path) {
        if (gpuTextures.contains(path))
            return gpuTextures[path];

        const auto result = registry.load(path);
        if (!result.has_value())
            throw CacheError("Failed to load asset: " + path.string());

        assetRefs[path] = result.value(); // hold the ref so registry can't evict it

        const auto bytes = result.value()->data();
        Image img = LoadImageFromMemory(".png",
            reinterpret_cast<const unsigned char*>(bytes.data()),
            static_cast<int>(bytes.size()));
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);

        gpuTextures[path] = tex;
        return tex;
    }

    void release(const fs::path& path) {
        std::cout << "Releasing: " << path << "\n";
        std::cout << "gpuTextures contains: " << gpuTextures.contains(path) << "\n";
        std::cout << "assetRefs contains: " << assetRefs.contains(path) << "\n";
        if (gpuTextures.contains(path)) {
            UnloadTexture(gpuTextures[path]);
            gpuTextures.erase(path);
        }
        assetRefs.erase(path);
    }

    ~TextureCache() {
        for (const auto& tex : gpuTextures | std::views::values) {
            std::cout << "Unloading GPU texture." << std::endl;
            UnloadTexture(tex);
        }
    }

    Texture2D peek(const fs::path& path) const {
        const fs::path canonical = fs::weakly_canonical(path);
        if (gpuTextures.contains(canonical))
            return gpuTextures.at(canonical);
        throw CacheError("peek() called on path not in GPU cache: " + path.string());
    }
};