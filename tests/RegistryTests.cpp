#include "gtest/gtest.h"
#include "Registry.h"
#include <fstream>
#include <filesystem>

// Helper: creates a temp file with known content, deleted after each test
class RegistryTest : public ::testing::Test {
protected:
    std::filesystem::path tempFile = "test_asset.bin";

    void SetUp() override {
        std::ofstream f(tempFile, std::ios::binary);
        f.write("helloworld", 10);
    }

    void TearDown() override {
        std::filesystem::remove(tempFile);
    }

    // Proxy method: RegistryTest is a friend, so it can call the private method
    bool DebugCanFitInCache(const std::filesystem::path& p) {
        return registry.CanFitInCache(p);
    }

    Registry registry;
};

// Loading a valid file should return a valid AssetRef
TEST_F(RegistryTest, LoadValidFileReturnsRef) {
    auto result = registry.Load(tempFile);
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(result->get(), nullptr);
}

// Loading a non-existent file should return nullopt
TEST_F(RegistryTest, LoadMissingFileReturnsNullopt) {
    auto result = registry.Load("does_not_exist.bin");
    EXPECT_FALSE(result.has_value());
}

// Loading an asset again should return a second asset ref
TEST_F(RegistryTest, LoadAssetAgainReturnsNewAssetRef) {
    auto firstRequest = registry.Load(tempFile);
    auto secondRequest = registry.Load(tempFile);

    // Both should have values
    ASSERT_TRUE(firstRequest.has_value());
    ASSERT_TRUE(secondRequest.has_value());

    // Should be distinct AssetRef objects (different pointers)
    EXPECT_NE(firstRequest->get(), secondRequest->get());
}

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

TEST_F(RegistryCacheTest, CanFitInCacheReturnsTrueWhenRegistryEmpty) {
    const auto smallFile = createSizedFile("small.cache_tmp", 100);
    EXPECT_TRUE(DebugCanFitInCache(smallFile));
}

TEST_F(RegistryCacheTest, ReturnsFalseWhenCacheIsAtCapacity) {
    const size_t capacity = registry.CACHE_CAPACITY;
    const auto bigFile = createSizedFile("full.cache_tmp", capacity);

    // Load it to consume the capacity
    registry.Load(bigFile);

    const auto anotherFile = createSizedFile("overflow.cache_tmp", 1);
    EXPECT_FALSE(DebugCanFitInCache(anotherFile));
}

TEST_F(RegistryCacheTest, ReturnsTrueWhenMultipleFilesFit) {
    const size_t quarterCap = registry.CACHE_CAPACITY / 4;
    const auto file1 = createSizedFile("file1.cache_tmp", quarterCap);
    const auto file2 = createSizedFile("file2.cache_tmp", quarterCap);

    registry.Load(file1);
    EXPECT_TRUE(DebugCanFitInCache(file2));
}

TEST_F(RegistryCacheTest, ReturnsTrueWhenSecondFileHitsCapacityExactly) {
    const size_t halfCap = registry.CACHE_CAPACITY / 2;
    const auto file1 = createSizedFile("half1.cache_tmp", halfCap);
    const auto file2 = createSizedFile("half2.cache_tmp", halfCap);

    registry.Load(file1);

    // Should be true because it's <= capacity, not strictly <
    EXPECT_TRUE(DebugCanFitInCache(file2));
}
