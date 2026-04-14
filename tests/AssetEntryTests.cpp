#include "gtest/gtest.h"
#include "AssetEntry.h"
#include "AssetRef.h"

// Helper to create an AssetEntry with known data
std::shared_ptr<AssetEntry> makeEntry(std::string_view content) {
    auto mem = std::make_unique<std::byte[]>(content.size());
    std::memcpy(mem.get(), content.data(), content.size());
    return std::make_shared<AssetEntry>(std::move(mem), content.size());
}

TEST(AssetEntryTest, CreateRefReturnsValidRef) {
    auto entry = makeEntry("hello");
    auto ref = entry->createRef();
    EXPECT_NE(ref, nullptr);
}

TEST(AssetEntryTest, EachRefGetsUniqueId) {
    auto entry = makeEntry("hello");
    auto ref1 = entry->createRef();
    auto ref2 = entry->createRef();
    EXPECT_NE(ref1, ref2);
}

TEST(AssetEntryTest, DataReturnsCorrectBytes) {
    auto entry = makeEntry("hello");
    auto data = entry->data();
    EXPECT_EQ(data.size(), 5);
    EXPECT_EQ(std::memcmp(data.data(), "hello", 5), 0);
}

TEST(AssetEntryTest, DataSizeMatchesInput) {
    auto entry = makeEntry("hello world");
    EXPECT_EQ(entry->data().size(), 11);
}

TEST(AssetEntryTest, TimeSinceLastRefFreedIsNulloptIfNoneFreed) {
    auto entry = makeEntry("hello");
    EXPECT_FALSE(entry->getTimeSinceLastRefFreed().has_value());
}

TEST(AssetEntryTest, TimeSinceLastRefFreedHasValueAfterFree) {
    auto entry = makeEntry("hello");
    auto ref = entry->createRef();
    entry->freeRef(*ref);
    EXPECT_TRUE(entry->getTimeSinceLastRefFreed().has_value());
}

TEST(AssetEntryTest, RefCountPrecision) {
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

TEST(AssetEntryTest, SpanMemoryStability) {
    const auto entry = makeEntry("stability_test");
    const auto data1 = entry->data().data();

    const auto ref = entry->createRef();
    const auto data2 = ref->data().data();

    // Verify we are looking at the exact same memory address (no copies)
    EXPECT_EQ(data1, data2);
}