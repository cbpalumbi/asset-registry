#include "gtest/gtest.h"
#include "Registry.h"
#include <fstream>
#include <filesystem>

// Helper: creates a temp file with known content, deleted after each test
class RegistryTest : public ::testing::Test {
protected:
    std::filesystem::path tempFile = "test_asset.bin";
    Registry registry;

    void SetUp() override {
        std::ofstream f(tempFile, std::ios::binary);
        f.write("helloworld", 10);
    }

    void TearDown() override {
        std::filesystem::remove(tempFile);
    }

    // Proxy method: RegistryTest is a friend, so it can call the private method
    bool DebugCanFitInCache(uintmax_t const fileSize) {
        return registry.canFitInCache(fileSize);
    }
    std::optional<std::vector<std::shared_ptr<AssetEntry>>> DebugTryGetEvictList(const uintmax_t fileSize) {
        return registry.tryGetEvictList(fileSize);
    }

    bool DebugEvictAssetByPath(fs::path const& path) {
        return registry.evictAssetByPath(path);
    }

    bool DebugHasEntryInCache(fs::path const& path) {
        return registry.hasEntryInCache(path);
    }
};

// Specialized fixture for creating tests with specific-sized files
class RegistryCacheTest : public RegistryTest {
protected:
    // Helper to create a file of a specific byte size
    static std::filesystem::path createSizedFile(const std::string& name, const size_t size) {
        std::filesystem::path p = std::filesystem::current_path() / name;
        std::ofstream f(p, std::ios::binary);
        if (size > 0) {
            f.seekp(size - 1);
            f.write("\0", 1);
        }
        f.close();
        return p;
    }

    void TearDown() override {
        RegistryTest::TearDown();
        // Cleanup any extra files created during tests
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (entry.path().extension() == ".cache_tmp") {
                std::filesystem::remove(entry.path());
            }
        }
    }
};

#pragma region LOAD TESTS

// Loading a valid file should return a valid AssetRef
TEST_F(RegistryTest, LoadValidFileReturnsRef) {
    const auto result = registry.load(tempFile);
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(result->get(), nullptr);
}

// Loading a non-existent file should return nullopt
TEST_F(RegistryTest, LoadMissingFileReturnsNullopt) {
    const auto result = registry.load("does_not_exist.bin");
    EXPECT_FALSE(result.has_value());
}

// Loading an asset again should return a second asset ref
TEST_F(RegistryTest, LoadAssetAgainReturnsNewAssetRef) {
    const auto firstRequest = registry.load(tempFile);
    const auto secondRequest = registry.load(tempFile);

    // Both should have values
    ASSERT_TRUE(firstRequest.has_value());
    ASSERT_TRUE(secondRequest.has_value());

    // Should be distinct AssetRef objects (different pointers)
    EXPECT_NE(firstRequest->get(), secondRequest->get());
}
#pragma endregion

#pragma region CACHE TESTS

TEST_F(RegistryCacheTest, CanFitInCacheReturnsTrueWhenRegistryEmpty) {
    const auto smallFile = createSizedFile("small.cache_tmp", 100);
    const auto smallFileSize = std::filesystem::file_size(smallFile);
    EXPECT_TRUE(DebugCanFitInCache(smallFileSize));
}

TEST_F(RegistryCacheTest, ReturnsFalseWhenCacheIsAtCapacity) {
    const size_t capacity = registry.CACHE_CAPACITY;
    const auto bigFile = createSizedFile("full.cache_tmp", capacity);

    // Load it to consume the capacity
    registry.load(bigFile);

    const auto anotherFile = createSizedFile("overflow.cache_tmp", 1);
    const auto anotherFileSize = std::filesystem::file_size(anotherFile);
    EXPECT_FALSE(DebugCanFitInCache(anotherFileSize));
}

TEST_F(RegistryCacheTest, ReturnsTrueWhenMultipleFilesFit) {
    const size_t quarterCap = registry.CACHE_CAPACITY / 4;
    const auto file1 = createSizedFile("file1.cache_tmp", quarterCap);
    const auto file2 = createSizedFile("file2.cache_tmp", quarterCap);

    registry.load(file1);
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCache(file2Size));
}

TEST_F(RegistryCacheTest, ReturnsTrueWhenSecondFileHitsCapacityExactly) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("half1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("half2.cache_tmp", halfCap);

    registry.load(file1);

    // Should be true because it's <= capacity, not strictly <
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCache(file2Size));
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsEmptyWhenNothingInRegistry) {
    const auto file = createSizedFile("evict_empty.cache_tmp", 100);
    const auto fileSize = std::filesystem::file_size(file);
    EXPECT_TRUE(DebugTryGetEvictList(fileSize)->empty());
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsNulloptWhenAllRefsHeld) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("held1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("needs_space.cache_tmp", halfCap + 1);

    // Hold the ref — keeps logical ref count at 1, not evictable
    auto ref = registry.load(file1);

    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_FALSE(DebugTryGetEvictList(file2Size).has_value());
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsEntriesWhenRefsReleased) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("evictable1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("needs_space.cache_tmp", halfCap);

    {
        auto ref = registry.load(file1); // ref count = 1
    } // AssetRef destroyed, freeRef called, ref count = 0

    const auto file2Size = std::filesystem::file_size(file2);
    const auto result = DebugTryGetEvictList(file2Size);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->empty());
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsNulloptWhenEvictableSpaceInsufficient) {
    const size_t quarterCap = registry.CACHE_CAPACITY / 4;
    const auto smallFile = createSizedFile("small_evictable.cache_tmp", quarterCap);
    const auto bigFile   = createSizedFile("too_big.cache_tmp", registry.CACHE_CAPACITY);

    {
        auto ref = registry.load(smallFile);
    } // evictable, but only quarterCap bytes free

    const auto bigFileSize = std::filesystem::file_size(bigFile);
    EXPECT_FALSE(DebugTryGetEvictList(bigFileSize).has_value());
}

TEST_F(RegistryCacheTest, TryGetEvictListExcludesEntriesWithLiveRefs) {
    const size_t thirdCap = registry.CACHE_CAPACITY / 3;
    const auto heldFile = createSizedFile("held_ref.cache_tmp",  thirdCap);
    const auto freeFile = createSizedFile("free_ref.cache_tmp",  thirdCap);
    const auto newFile  = createSizedFile("incoming.cache_tmp",  thirdCap);

    auto heldRef = registry.load(heldFile); // stays in scope, ref count = 1

    {
        auto tempRef = registry.load(freeFile);
    } // ref count drops to 0, freeFile is evictable

    const auto newFileSize = std::filesystem::file_size(newFile);
    const auto result = DebugTryGetEvictList(newFileSize);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 1u); // only freeFile, not heldFile
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsSufficientSubsetNotAllEntries) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1   = createSizedFile("evict_a.cache_tmp", halfCap);
    const auto file2   = createSizedFile("evict_b.cache_tmp", halfCap);
    const auto newFile = createSizedFile("small_new.cache_tmp", 100);

    {
        auto ref1 = registry.load(file1);
    }
    {
        auto ref2 = registry.load(file2);
    }

    const auto newFileSize = std::filesystem::file_size(newFile);
    const auto result = DebugTryGetEvictList(newFileSize);
    ASSERT_TRUE(result.has_value());
    // 100 bytes needed — one halfCap entry is more than sufficient
    // early exit should mean only 1 entry collected, not both
    EXPECT_EQ(result->size(), 1u);
}
#pragma endregion

#pragma region EVICT TESTS

TEST_F(RegistryCacheTest, EvictedAssetIsRemovedFromEntries) {
    const auto file = createSizedFile("evict_remove.cache_tmp", 100);
    { auto ref = registry.load(file); }

    DebugEvictAssetByPath(file);

    EXPECT_FALSE(DebugHasEntryInCache(file));
}

TEST_F(RegistryCacheTest, EvictedAssetReducesCurrentUsage) {
    const auto file = createSizedFile("evict_usage.cache_tmp", 100);
    { const auto ref = registry.load(file); }

    const auto usageBefore = registry.getCurrentUsage();
    DebugEvictAssetByPath(file);
    const auto usageAfter = registry.getCurrentUsage();

    EXPECT_LT(usageAfter, usageBefore);
}

TEST_F(RegistryCacheTest, EvictedAssetReducesCurrentUsage2) {
    const auto file = createSizedFile("evict_usage.cache_tmp", 100);
    uint32_t usageBefore = 0;
    {
        const auto ref = registry.load(file);
        usageBefore = registry.getCurrentUsage();
    }

    DebugEvictAssetByPath(file);
    const auto usageAfter = registry.getCurrentUsage();

    EXPECT_LT(usageAfter, usageBefore);
}

TEST_F(RegistryCacheTest, LoadAfterEvictReloadsAsset) {
    const auto file = createSizedFile("evict_reload.cache_tmp", 100);
    { const auto ref = registry.load(file); }

    DebugEvictAssetByPath(file);
    const auto ref = registry.load(file);

    ASSERT_TRUE(ref.has_value());
    EXPECT_NE(*ref, nullptr);
}

TEST_F(RegistryCacheTest, EvictionMakesRoomForNewAsset) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("evict_room_a.cache_tmp", halfCap);
    const auto file2 = createSizedFile("evict_room_b.cache_tmp", halfCap);

    { auto ref1 = registry.load(file1); }
    { auto ref2 = registry.load(file2); }  // triggers eviction of file1

    EXPECT_FALSE(DebugHasEntryInCache(file1));
    EXPECT_TRUE(DebugHasEntryInCache(file2));
}

TEST_F(RegistryCacheTest, EvictingNonExistentPathReturnsFalse) {
    const auto result = DebugEvictAssetByPath("does_not_exist.cache_tmp");
    EXPECT_FALSE(result);
}

TEST_F(RegistryCacheTest, CacheUsageIsZeroAfterEvictingAllEntries) {
    const auto file1 = createSizedFile("evict_all_a.cache_tmp", 100);
    const auto file2 = createSizedFile("evict_all_b.cache_tmp", 100);
    { const auto ref1 = registry.load(file1); }
    { const auto ref2 = registry.load(file2); }

    DebugEvictAssetByPath(file1);
    DebugEvictAssetByPath(file2);

    EXPECT_EQ(registry.getCurrentUsage(), 0u);
}

#pragma endregion
