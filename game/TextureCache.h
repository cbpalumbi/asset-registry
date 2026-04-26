#pragma once
#include "raylib.h"
#include "Registry.h"
#include "CacheError.h"
#include "AssetRef.h"

#include <unordered_map>
#include <filesystem>
#include <ranges>

namespace fs = std::filesystem;

// Go-between for raylib and AssetRegistry
class TextureCache {
    Registry& registry;
    std::unordered_map<fs::path, Texture2D> gpuTextures;

public:
    explicit TextureCache(Registry& registry) : registry(registry) {}

    Texture2D get(const fs::path& path) {
        if (gpuTextures.contains(path))
            return gpuTextures[path];

        const auto result = registry.load(path);
        if (!result.has_value())
            throw CacheError("Failed to load asset: " + path.string());

        auto bytes = result.value()->data();

        Image img = LoadImageFromMemory(".png",
            reinterpret_cast<const unsigned char *>(bytes.data()), (int)bytes.size());
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);

        gpuTextures[path] = tex;
        return tex;
    }

    ~TextureCache() {
        for (const auto &tex: gpuTextures | std::views::values)
            UnloadTexture(tex);
    }
};