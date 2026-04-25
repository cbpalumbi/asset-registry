#include "gtest/gtest.h"
#include "AssetEntry.h"
#include "AssetRef.h"

class AssetEntryTest : public ::testing::Test {
protected:
    static std::shared_ptr<AssetEntry> makeEntry(std::string_view content) {
        auto mem = std::make_unique<std::byte[]>(content.size());
        std::memcpy(mem.get(), content.data(), content.size());
        return std::make_shared<AssetEntry>("fake/path.asset", std::move(mem), content.size(), nullptr);
    }

    static std::shared_ptr<AssetEntry> makeSizedEntry(const size_t size) {
        auto mem = std::make_unique<std::byte[]>(size);
        std::memset(mem.get(), 0xAB, size);  // fill with known pattern
        return std::make_shared<AssetEntry>("fake/path.asset", std::move(mem), size, nullptr);
    }
};

#pragma region CreateRef

TEST_F(AssetEntryTest, CreateRef_WithValidEntry_ReturnsRef) {
    const auto entry = makeEntry("hello");
    const auto ref = entry->createRef();
    EXPECT_NE(ref, nullptr);
}

TEST_F(AssetEntryTest, CreateRef_WithMultipleRefs_EachRefGetsUniqueId) {
    const auto entry = makeEntry("hello");
    const auto ref1 = entry->createRef();
    const auto ref2 = entry->createRef();
    EXPECT_NE(ref1, ref2);
}

#pragma endregion

#pragma region GetData

TEST_F(AssetEntryTest, GetData_WithValidEntry_ReturnsCorrectData) {
    const auto entry = makeEntry("hello");
    const auto data = entry->data();
    EXPECT_EQ(std::memcmp(data.data(), "hello", 5), 0);
}

TEST_F(AssetEntryTest, GetData_WithValidEntry_ReturnsDataOfCorrectSize) {
    const auto entry = makeEntry("hello");
    const auto data = entry->data();
    EXPECT_EQ(data.size(), 5);
}

TEST_F(AssetEntryTest, GetData_WithValidEntry_ReturnsSameMemoryAddress) {
    const auto entry = makeEntry("stability_test");
    const auto data1 = entry->data().data();

    const auto ref = entry->createRef();
    const auto data2 = ref->data().data();

    // Verify we are looking at the exact same memory address (no copies)
    EXPECT_EQ(data1, data2);
}

#pragma endregion

#pragma region GetRefCount

TEST_F(AssetEntryTest, GetRefCount_AfterRefFreed_ReturnsCorrectCount) {
    const auto entry = makeEntry("ref_test");
    EXPECT_EQ(entry->getRefCount(), 0);

    {
        const auto ref1 = entry->createRef();
        const auto ref2 = entry->createRef();
        EXPECT_EQ(entry->getRefCount(), 2);

        entry->freeRef(*ref1);
        EXPECT_EQ(entry->getRefCount(), 1);
    } // ref2 destructor will call freeRef again via RAII

    EXPECT_EQ(entry->getRefCount(), 0);
}

#pragma endregion

#pragma region InvalidateRefs

TEST_F(AssetEntryTest, InvalidateRefs_WithActiveRefs_InvalidatesAllActiveRefs) {
    auto entry = makeEntry("hello");
    auto ref1 = entry->createRef();
    auto ref2 = entry->createRef();

    entry->invalidateRefs();

    EXPECT_THROW(ref1->data(), std::runtime_error);
    EXPECT_THROW(ref2->data(), std::runtime_error);
}

TEST_F(AssetEntryTest, InvalidateRefs_WithNoActiveRefs_DoesNotThrow) {
    auto entry = makeEntry("hello");
    EXPECT_NO_THROW(entry->invalidateRefs());
}

TEST_F(AssetEntryTest, InvalidateRefs_WithExpiredRefs_SkipsExpiredRefs) {
    auto entry = makeEntry("hello");
    {
        auto ref = entry->createRef();
    } // ref destroyed here, weak_ptr expired

    // should not crash iterating over expired weak_ptrs
    EXPECT_NO_THROW(entry->invalidateRefs());
}

TEST_F(AssetEntryTest, InvalidateRefs_WithActiveRef_DoesNotNullThePointer) {
    // the shared_ptr should still be valid even though the contents are nulled
    auto entry = makeEntry("hello");
    auto ref = entry->createRef();

    entry->invalidateRefs();

    EXPECT_NE(ref, nullptr);  // object still alive, just invalidated
}

#pragma endregion