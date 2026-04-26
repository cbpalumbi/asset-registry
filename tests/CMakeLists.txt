cmake_minimum_required(VERSION 3.25)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(AssetRegistryTests
        AssetEntryTests.cpp
        AssetRefTests.cpp
        RegistryTests.cpp
)

target_link_libraries(AssetRegistryTests PRIVATE AssetRegistryLib GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(AssetRegistryTests)