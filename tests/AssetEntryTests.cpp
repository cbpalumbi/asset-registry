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

    void TearDown() override {
        // nothing to clean up — no real files created
    }
};

TEST_F(AssetEntryTest, CreateRefReturnsValidRef) {
    auto entry = makeEntry("hello");
    auto ref = entry->createRef();
    EXPECT_NE(ref, nullptr);
}

TEST_F(AssetEntryTest, EachRefGetsUniqueId) {
    auto entry = makeEntry("hello");
    auto ref1 = entry->createRef();
    auto ref2 = entry->createRef();
    EXPECT_NE(ref1, ref2);
}

TEST_F(AssetEntryTest, DataReturnsCorrectBytes) {
    auto entry = makeEntry("hello");
    auto data = entry->data();
    EXPECT_EQ(data.size(), 5);
    EXPECT_EQ(std::memcmp(data.data(), "hello", 5), 0);
}

TEST_F(AssetEntryTest, DataSizeMatchesInput) {
    auto entry = makeEntry("hello world");
    EXPECT_EQ(entry->data().size(), 11);
}

// TODO: remove
TEST_F(AssetEntryTest, TimeSinceLastRefFreedIsNulloptIfNoneFreed) {
    auto entry = makeEntry("hello");
    EXPECT_FALSE(entry->getTimeSinceLastRefFreed().has_value());
}

TEST_F(AssetEntryTest, RefCountPrecision) {
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

TEST_F(AssetEntryTest, SpanMemoryStability) {
    const auto entry = makeEntry("stability_test");
    const auto data1 = entry->data().data();

    const auto ref = entry->createRef();
    const auto data2 = ref->data().data();

    // Verify we are looking at the exact same memory address (no copies)
    EXPECT_EQ(data1, data2);
}

#pragma region INVALIDATION TESTS

TEST_F(AssetEntryTest, InvalidateRefsInvalidatesAllActiveRefs) {
    auto entry = makeEntry("hello");
    auto ref1 = entry->createRef();
    auto ref2 = entry->createRef();

    entry->invalidateRefs();

    EXPECT_THROW(ref1->data(), std::runtime_error);
    EXPECT_THROW(ref2->data(), std::runtime_error);
}


TEST_F(AssetEntryTest, InvalidateRefsDoesNotCrashWithNoRefs) {
    auto entry = makeEntry("hello");
    EXPECT_NO_THROW(entry->invalidateRefs());
}

TEST_F(AssetEntryTest, InvalidateRefsSkipsExpiredRefs) {
    auto entry = makeEntry("hello");
    {
        auto ref = entry->createRef();
    } // ref destroyed here, weak_ptr expired

    // should not crash iterating over expired weak_ptrs
    EXPECT_NO_THROW(entry->invalidateRefs());
}

TEST_F(AssetEntryTest, RefRemainsAliveAfterInvalidation) {
    // the shared_ptr should still be valid even though the contents are nulled
    auto entry = makeEntry("hello");
    auto ref = entry->createRef();

    entry->invalidateRefs();

    EXPECT_NE(ref, nullptr);  // object still alive, just invalidated
}

#pragma endregion