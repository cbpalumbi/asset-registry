#include "gtest/gtest.h"
#include "AssetRef.h"
#include "Registry.h"
#include <fstream>
#include <filesystem>

class AssetRefTests : public ::testing::Test {
protected:
    std::filesystem::path tempFile = "test_asset.bin";

    void SetUp() override {
        std::ofstream f(tempFile, std::ios::binary);
        f.write("helloworld", 10);
    }

    void TearDown() override {
        std::filesystem::remove(tempFile);
    }

    static std::shared_ptr<AssetEntry> DebugGetEntry(const AssetRef& ref) {
        return ref.assetEntry;
    }

    Registry registry;
};

// --- RAII Destructor ---

// When all refs are dropped, the entry's ref map should be empty
TEST_F(AssetRefTests, DestructorRemovesRefFromEntry) {
    { // enter scope
        const auto ref = registry.load(tempFile);
        ASSERT_TRUE(ref.has_value());

        // using protected helper instead of direct member access
        const auto entry = DebugGetEntry(**ref);
        EXPECT_EQ(entry->getRefCount(), 1);
    } // exit scope, ref destroyed here
}

// Destroying one ref should not affect other live refs to the same asset
TEST_F(AssetRefTests, DestructorOnlyRemovesOwnRef) {
    const auto keeper = registry.load(tempFile);
    ASSERT_TRUE(keeper.has_value());
    const auto keeperEntry = DebugGetEntry(**keeper);

    {
        const auto temp = registry.load(tempFile);
        ASSERT_TRUE(temp.has_value());
        EXPECT_EQ(keeperEntry->getRefCount(), 2);
    } // only temp destroyed

    EXPECT_EQ(keeperEntry->getRefCount(), 1);
}

// Both refs should point to the same AssetEntry object
TEST_F(AssetRefTests, MultipleLoadsShareSameEntry) {
    const auto ref1 = registry.load(tempFile);
    const auto ref2 = registry.load(tempFile);

    ASSERT_TRUE(ref1.has_value() && ref2.has_value());

    // Ensure both point to the same underlying AssetEntry address
    EXPECT_EQ(DebugGetEntry(**ref1).get(), DebugGetEntry(**ref2).get());
}