#include "gtest/gtest.h"
#include "Registry.h"
#include "CacheError.h"

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
        
        // Cleanup any extra files created during tests
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (entry.path().extension() == ".cache_tmp") {
                std::filesystem::remove(entry.path());
            }
        }
    }

    // Proxy method: RegistryTest is a friend, so it can call the private method
    bool DebugCanFitInCacheWithEviction(uint64_t const fileSize) const {
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

};

#pragma region Load

TEST_F(RegistryTest, Load_ValidFile_ReturnsRef) {
    const auto result = registry.load(tempFile);
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(result->get(), nullptr);
}

TEST_F(RegistryTest, Load_MissingFile_ThrowsCacheError) {
   EXPECT_THROW(registry.load("does_not_exist.bin"), AssetNotFoundError);
}

TEST_F(RegistryTest, Load_SameAssetAgain_ReturnsNewAssetRef) {
    const auto firstRequest = registry.load(tempFile);
    const auto secondRequest = registry.load(tempFile);

    // Both should have values
    ASSERT_TRUE(firstRequest.has_value());
    ASSERT_TRUE(secondRequest.has_value());

    // Should be distinct AssetRef objects (different pointers)
    EXPECT_NE(firstRequest->get(), secondRequest->get());
}

TEST_F(RegistryTest, Load_WithTooLargeFile_ReturnsNullopt) {
    const auto file = createSizedFile("file1.cache_tmp", registry.CACHE_CAPACITY + 1);
    EXPECT_THROW(registry.load(file), AssetSizeExceedsCacheCapacityError);
}

TEST_F(RegistryTest, Load_AssetWithSizeEqualToCacheCapacity_Succeeds) {
    const auto file1 = createSizedFile("file1.cache_tmp", registry.CACHE_CAPACITY);

    const auto ref1 = registry.load(file1);

    ASSERT_TRUE(ref1.has_value());
}

TEST_F(RegistryTest, Load_WhenEvictionImpossible_ReturnsNullopt) {
    const auto file1 = createSizedFile("file1.cache_tmp", registry.CACHE_CAPACITY);
    const auto file2 = createSizedFile("file2.cache_tmp", 1);

    const auto ref1 = registry.load(file1);

    EXPECT_THROW(registry.load(file2), NoSpaceInCacheError);
}

TEST_F(RegistryTest, Load_WithAssetEviction_EvictedAssetIsRemovedFromEntries) {
    const auto file = createSizedFile("evict_remove.cache_tmp", 100);
    { auto ref = registry.load(file); }

    DebugEvictAssetByPath(file);

    EXPECT_FALSE(DebugHasEntryInCache(file));
}

TEST_F(RegistryTest, Load_SameAssetAfterEviction_ReloadsAsset) {
    const auto file = createSizedFile("evict_reload.cache_tmp", 100);
    { const auto ref = registry.load(file); }

    DebugEvictAssetByPath(file);
    const auto ref = registry.load(file);

    ASSERT_TRUE(ref.has_value());
    EXPECT_NE(*ref, nullptr);
}

TEST_F(RegistryTest, Load_WithEviction_LoadsNewAsset) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("evict_room_a.cache_tmp", halfCap);
    const auto file2 = createSizedFile("evict_room_b.cache_tmp", halfCap + 1);

    { auto ref1 = registry.load(file1); }
    { auto ref2 = registry.load(file2); }  // triggers eviction of file1

    EXPECT_FALSE(DebugHasEntryInCache(file1));
    EXPECT_TRUE(DebugHasEntryInCache(file2));
}

TEST_F(RegistryTest, Load_WithEviction_EvictsLeastRecentlyFreedAsset) {
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

TEST_F(RegistryTest, Load_WithMultipleEvictionsNeeded_EvictsOnlyEnoughToFitNewAsset) {
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

TEST_F(RegistryTest, Load_WithEviction_EvictsLeastRecentlyFreedFirst) {
    // fill cache with three third-sized assets
    const size_t thirdCap = registry.CACHE_CAPACITY / 3;
    const auto fileA = createSizedFile("evict_order_a.cache_tmp", thirdCap);
    const auto fileB = createSizedFile("evict_order_b.cache_tmp", thirdCap);
    const auto fileC = createSizedFile("evict_order_c.cache_tmp", thirdCap);

    // load all three, then free in order A, B, C
    // so A is least recently freed (back of lruList), C is most recently freed (front)
    { const auto ref = registry.load(fileA); }
    { const auto ref = registry.load(fileB); }
    { const auto ref = registry.load(fileC); }

    // verify lruList order before eviction: front=C, back=A
    {
        const auto evictable = DebugTryGetEvictableList();
        ASSERT_EQ(evictable.size(), 3u);
        EXPECT_EQ(evictable.front(), fileC); // most recently freed
        EXPECT_EQ(evictable.back(),  fileA); // least recently freed
    }

    // now load a third-sized asset — only needs to evict one (A, the oldest)
    const auto fileD = createSizedFile("evict_order_d.cache_tmp", thirdCap);
    const auto ref = registry.load(fileD);

    // A should be gone, B and C should still be in the evictable list
    const auto evictable = DebugTryGetEvictableList();
    EXPECT_EQ(evictable.size(), 2u);
    EXPECT_EQ(evictable.front(), fileC); // C still most recently freed
    EXPECT_EQ(evictable.back(),  fileB); // B now least recently freed

    // A should no longer be in the registry at all
    EXPECT_FALSE(DebugHasEntryInCache(fileA));
    EXPECT_TRUE(DebugHasEntryInCache(fileB));
    EXPECT_TRUE(DebugHasEntryInCache(fileC));
    EXPECT_TRUE(DebugHasEntryInCache(fileD));
}

#pragma endregion

#pragma region CanFitInCacheWithEviction

TEST_F(RegistryTest, CanFitInCacheWithEviction_WhenRegistryEmpty_ReturnsTrue) {
    const auto smallFile = createSizedFile("small.cache_tmp", 100);
    const auto smallFileSize = std::filesystem::file_size(smallFile);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(smallFileSize));
}

TEST_F(RegistryTest, CanFitInCacheWithEviction_WhenMultipleFilesCanFit_ReturnsTrue) {
    const size_t quarterCap = registry.CACHE_CAPACITY / 4;
    const auto file1 = createSizedFile("file1.cache_tmp", quarterCap);
    const auto file2 = createSizedFile("file2.cache_tmp", quarterCap);

    registry.load(file1);
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(file2Size));
}

TEST_F(RegistryTest, CanFitInCacheWithEviction_WhenNewFileHitsCapacityExactly_ReturnsTrue) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("half1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("half2.cache_tmp", halfCap);

    registry.load(file1);

    // Should be true because it's <= capacity, not strictly <
    const auto file2Size = std::filesystem::file_size(file2);
    EXPECT_TRUE(DebugCanFitInCacheWithEviction(file2Size));
}

#pragma endregion

#pragma region TryGetEvictableList

TEST_F(RegistryTest, TryGetEvictableList_WhenNothingInRegistry_ReturnsEmpty) {
    const auto file = createSizedFile("evict_empty.cache_tmp", 100);
    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryTest, TryGetEvictableList_WhenAllRefsHeld_ReturnsEmpty) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("held1.cache_tmp", halfCap);

    // Hold the ref — keeps logical ref count at 1, not evictable
    auto ref = registry.load(file1);

    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryTest, TryGetEvictableList_WhenAllRefsHaveBeenReleased_ReturnsEmpty) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("evictable1.cache_tmp", halfCap);

    {
        auto ref = registry.load(file1); // ref count = 1
    } // AssetRef destroyed, freeRef called, ref count = 0

    const auto result = DebugTryGetEvictableList();
    EXPECT_FALSE(result.empty());
}

TEST_F(RegistryTest, TryGetEvictableList_WithLiveRefs_ExcludesThoseEntries) {
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

TEST_F(RegistryTest, TryGetEvictableList_AfterRefReleased_ListContainsEntry) {
    const std::string path = "lru_released.cache_tmp";
    const auto file1 = createSizedFile(path, 100);
    {
        const auto ref = registry.load(file1);
    } // ref released, entry should enter lru list

    const auto result = DebugTryGetEvictableList();
    const std::filesystem::path full_path = std::filesystem::current_path() / path;
    EXPECT_EQ(result.front(), full_path);
}

TEST_F(RegistryTest, TryGetEvictableList_WhenEvictedAssetWasReReguested_ListDoesNotContainEntry) {
    const auto file1 = createSizedFile("lru_re_requested.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // ref released, entry enters lru list

    // re-request the asset — should be removed from lru list
    const auto ref2 = registry.load(file1);
    EXPECT_TRUE(DebugTryGetEvictableList().empty());
}

TEST_F(RegistryTest, TryGetEvictableList_WhenRefReleasedThenReloadedThenReleased_ListContainsEntry) {
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

TEST_F(RegistryTest, TryGetEvictableList_WhenRefReleasedThenReloadedThenMovedThenReleased_ListContainsEntry) {
    const std::string path = "lru.cache_tmp";
    const auto file1 = createSizedFile(path, 100);
    {
        const auto ref = registry.load(file1);
    } // enters lru list

    auto ref2 = registry.load(file1); // removed from lru list
    // ref2 goes out of scope, should re-enter lru list

    // need to release ref2 here to trigger freeRef
    { const auto scoped = std::move(ref2); }

    const auto result = DebugTryGetEvictableList();
    const std::filesystem::path full_path = std::filesystem::current_path() / path;
    EXPECT_EQ(result.front(), full_path);
}

TEST_F(RegistryTest, TryGetEvictableList_WithMultipleAssetsFreed_LeastRecentlyFreedAssetAtBackOfList) {
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

    const std::filesystem::path path_a = std::filesystem::current_path() / path_a_name;
    const std::filesystem::path path_b = std::filesystem::current_path() / path_b_name;

    EXPECT_EQ(result.front(), path_b); // fileB, most recently freed
    EXPECT_EQ(result.back(), path_a);  // fileA, least recently freed
}

#pragma endregion

#pragma region EvictAssetByPath

TEST_F(RegistryTest, EvictAssetByPath_WithAssetEviction_ChangesCurrentUsage) {
    // This version retrieves the cache usage after the asset ref is released
    const auto file = createSizedFile("evict_usage.cache_tmp", 100);
    { const auto ref = registry.load(file); }

    const auto usageBefore = registry.getCurrentUsage();
    DebugEvictAssetByPath(file);
    const auto usageAfter = registry.getCurrentUsage();

    EXPECT_LT(usageAfter, usageBefore);
}

TEST_F(RegistryTest, EvictAssetByPath_WithAssetEviction_EvictedAssetReducesCurrentUsage2) {
    // This version retrieves the cache usage while the asset ref is still held
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

TEST_F(RegistryTest, EvictAssetByPath_WithNonExistentPath_ReturnsFalse) {
    const auto result = DebugEvictAssetByPath("does_not_exist.cache_tmp");
    EXPECT_FALSE(result);
}

TEST_F(RegistryTest, EvictAssetByPath_AllAssetsEvicted_CacheUsageIsZero) {
    const auto file1 = createSizedFile("evict_all_a.cache_tmp", 100);
    const auto file2 = createSizedFile("evict_all_b.cache_tmp", 100);
    { const auto ref1 = registry.load(file1); }
    { const auto ref2 = registry.load(file2); }

    DebugEvictAssetByPath(file1);
    DebugEvictAssetByPath(file2);

    EXPECT_EQ(registry.getCurrentUsage(), 0u);
}

TEST_F(RegistryTest, EvictAssetByPath_WithPreviouslyFreedAsset_RemovesEntryFromCache) {
    const auto file1 = createSizedFile("lru_evicted.cache_tmp", 100);
    {
        const auto ref = registry.load(file1);
    } // enters lru list

    DebugEvictAssetByPath(file1);

    // entry is gone entirely, not just removed from lru list
    EXPECT_FALSE(DebugHasEntryInCache(file1));
}

#pragma endregion