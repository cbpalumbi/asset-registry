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
