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
    bool DebugCanFitInCacheWithEviction(uintmax_t const fileSize) const {
        return registry.canFitInCacheWithEviction(fileSize);
    }
    std::list<fs::path> DebugTryGetEvictableList() {
        return registry.tryGetEvictableList();
    }

    bool DebugEvictAssetByPath(fs::path const& path) {
        return registry.evictAssetByPath(path);
    }

    bool DebugHasEntryInCache(fs::path const& path) {
        return registry.hasEntryInCache(path);
    }

    std::optional<std::shared_ptr<AssetEntry>> DebugGetEntryByPath(fs::path const& path) {
        return registry.getEntryByPath(path);
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
TEST_F(RegistryCacheTest, LoadAssetWithSizeEqualToCacheCapacitySucceeds) {
    const auto file1 = createSizedFile("file1.cache_tmp", registry.CACHE_CAPACITY);

    const auto ref1 = registry.load(file1);

    ASSERT_TRUE(ref1.has_value());
}

// Loading an asset when the cache cannot evict anything returns nullopt
TEST_F(RegistryCacheTest, LoadAssetWhenEvictionImpossibleReturnsNullopt) {
    const auto file1 = createSizedFile("file1.cache_tmp", registry.CACHE_CAPACITY);
    const auto file2 = createSizedFile("file2.cache_tmp", 1);

    const auto ref1 = registry.load(file1);
    const auto ref2 = registry.load(file2);

    ASSERT_FALSE(ref2.has_value());
}

TEST_F(RegistryCacheTest, CanFitInCacheWithEvictionReturnsTrueWhenRegistryEmpty) {
    const auto smallFile = createSizedFile("small.cache_tmp", 100);
    const auto smallFileSize = std::filesystem::file_size(smallFile);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(smallFileSize));
}

TEST_F(RegistryCacheTest, CanFitInCacheWithEvictionReturnsTrueWhenMultipleFilesFit) {
    const size_t quarterCap = registry.CACHE_CAPACITY / 4;
    const auto file1 = createSizedFile("file1.cache_tmp", quarterCap);
    const auto file2 = createSizedFile("file2.cache_tmp", quarterCap);

    registry.load(file1);
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(file2Size));
}

TEST_F(RegistryCacheTest, CanFitInCacheWithEvictionReturnsTrueWhenSecondFileHitsCapacityExactly) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("half1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("half2.cache_tmp", halfCap);

    registry.load(file1);

    // Should be true because it's <= capacity, not strictly <
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(file2Size));
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsEmptyWhenNothingInRegistry) {
    const auto file = createSizedFile("evict_empty.cache_tmp", 100);
    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsEmptyWhenAllRefsHeld) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("held1.cache_tmp", halfCap);

    // Hold the ref — keeps logical ref count at 1, not evictable
    auto ref = registry.load(file1);

    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryCacheTest, TryGetEvictListReturnsEmptyWhenRefsReleased) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("evictable1.cache_tmp", halfCap);

    {
        auto ref = registry.load(file1); // ref count = 1
    } // AssetRef destroyed, freeRef called, ref count = 0

    const auto result = DebugTryGetEvictableList();
    EXPECT_FALSE(result.empty());
}

TEST_F(RegistryCacheTest, TryGetEvictListExcludesEntriesWithLiveRefs) {
    const size_t thirdCap = registry.CACHE_CAPACITY / 3;
    const auto heldFile = createSizedFile("held_ref.cache_tmp",  thirdCap);
    const auto freeFile = createSizedFile("free_ref.cache_tmp",  thirdCap);

    auto heldRef = registry.load(heldFile); // stays in scope, ref count = 1

    {
        auto tempRef = registry.load(freeFile);
    } // ref count drops to 0, freeFile is evictable

    const auto result = DebugTryGetEvictableList();

    EXPECT_EQ(result.size(), 1u); // only freeFile, not heldFile
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
    const auto file2 = createSizedFile("evict_room_b.cache_tmp", halfCap + 1);

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

#pragma region LRU TESTS

TEST_F(RegistryCacheTest, LruListIsEmptyWhenAllRefsHeld) {
    const auto file1 = createSizedFile("lru_held.cache_tmp", 100);
    const auto ref = registry.load(file1);

    // ref is still alive, entry should not be on the lru list
    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryCacheTest, LruListContainsEntryAfterRefReleased) {
    const auto file1 = createSizedFile("lru_released.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // ref released, entry should enter lru list

    const auto result = DebugTryGetEvictableList();
    EXPECT_EQ(result.size(), 1u);
}

TEST_F(RegistryCacheTest, LruListEntryRemovedWhenAssetReRequested) {
    const auto file1 = createSizedFile("lru_re_requested.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // ref released, entry enters lru list

    // re-request the asset — should be removed from lru list
    const auto ref2 = registry.load(file1);
    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryCacheTest, LruListEntryReAddedAfterSecondRefReleased) {
    const auto file1 = createSizedFile("lru_re_added.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // enters lru list

    auto ref2 = registry.load(file1); // removed from lru list
    // ref2 goes out of scope, should re-enter lru list

    // need to release ref2 here to trigger freeRef
    { const auto scoped = std::move(ref2); }

    const auto result = DebugTryGetEvictableList();
    EXPECT_EQ(result.size(), 1u);
}

TEST_F(RegistryCacheTest, LruListEntryReAddedAfterSecondRefReleased2) {
    const auto file1 = createSizedFile("lru_re_added.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // enters lru list

    {
        const auto ref2 = registry.load(file1); // removed from lru list
    }// ref2 goes out of scope, should re-enter lru list

    const auto result = DebugTryGetEvictableList();
    EXPECT_EQ(result.size(), 1u);
}

TEST_F(RegistryCacheTest, LruListHasLeastRecentlyFreedAssetAtBackOfList) {
    const size_t thirdCap = registry.CACHE_CAPACITY / 3;
    const std::string path_a_name = "lru_order_a.cache_tmp";
    const std::string path_b_name = "lru_order_b.cache_tmp";
    const auto fileA = createSizedFile(path_a_name, thirdCap);
    const auto fileB = createSizedFile(path_b_name, thirdCap);

    // release fileA first, then fileB
    // fileA should be at the back of the list (evicted first)
    { const auto refA = registry.load(fileA); }
    { const auto refB = registry.load(fileB); }

    const auto result = DebugTryGetEvictableList();
    auto back = result.back();

    const std::filesystem::path path_a = std::filesystem::current_path() / path_a_name;
    const std::filesystem::path path_b = std::filesystem::current_path() / path_b_name;

    EXPECT_EQ(result.front(), path_b); // fileB, most recently freed
    EXPECT_EQ(result.back(), path_a);  // fileA, least recently freed
}

TEST_F(RegistryCacheTest, RegistryEvictsLeastRecentlyFreedAsset) {
    const size_t thirdCap = registry.CACHE_CAPACITY / 3;
    const std::string path_a_name = "lru_order_a.cache_tmp";
    const std::string path_b_name = "lru_order_b.cache_tmp";
    const auto fileA = createSizedFile(path_a_name, thirdCap);
    const auto fileB = createSizedFile(path_b_name, thirdCap);

    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto fileC = createSizedFile("lru_order_c.cache_tmp", halfCap);

    // release fileA first, then fileB
    { const auto refA = registry.load(fileA); }
    { const auto refB = registry.load(fileB); }

    const auto ref = registry.load(fileC);
    const auto result = DebugTryGetEvictableList();

    const std::filesystem::path path_b = std::filesystem::current_path() / path_b_name;

    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result.front(), path_b); // fileB, most recently freed
    EXPECT_EQ(result.back(), path_b);  // fileA, least recently freed
}

TEST_F(RegistryCacheTest, RegistryEvictsOnlyEnoughToFitNewAsset) {
    const size_t fifthCap = registry.CACHE_CAPACITY / 5;
    const auto fileA = createSizedFile("itemA.cache_tmp", fifthCap);
    const auto fileB = createSizedFile("itemB.cache_tmp", fifthCap);
    const auto fileC = createSizedFile("itemC.cache_tmp", fifthCap);
    const std::string fileDName = "itemD.cache_tmp";
    const auto fileD = createSizedFile(fileDName, fifthCap);
    const std::string fileEName = "itemE.cache_tmp";
    const auto fileE = createSizedFile(fileEName, fifthCap);

    // release fileA first, then fileB
    { const auto refA = registry.load(fileA); }
    { const auto refB = registry.load(fileB); }
    { const auto refC = registry.load(fileC); }
    { const auto refD = registry.load(fileD); }
    { const auto refE = registry.load(fileE); }

    const size_t fourFifthsCap = registry.CACHE_CAPACITY * 3 / 5;
    const auto fileF = createSizedFile("itemF.cache_tmp", fourFifthsCap);

    const auto ref = registry.load(fileF);
    const auto result = DebugTryGetEvictableList();

    const std::filesystem::path path_d = std::filesystem::current_path() / fileDName;
    const std::filesystem::path path_e = std::filesystem::current_path() / fileEName;


    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result.front(), path_e); // fileE, most recently freed
    EXPECT_EQ(result.back(), path_d);  // fileD, least recently freed (of those still in the registry)
}

TEST_F(RegistryCacheTest, LruEntryRemovedFromListOnEviction) {
    const auto file1 = createSizedFile("lru_evicted.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // enters lru list

    DebugEvictAssetByPath(file1);

    // entry is gone entirely, not just removed from lru list
    EXPECT_FALSE(DebugHasEntryInCache(file1));
}

#pragma endregion