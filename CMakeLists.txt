cmake_minimum_required(VERSION 3.20)
project(AssetRegistry)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
    add_compile_options(/fsanitize=address)
    add_compile_options(/Zi)
    add_link_options(/DEBUG)
else()
    add_compile_options(-Wall -Wextra)
endif()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(AssetRegistry main.cpp
        Registry.cpp
        Registry.h
        AssetEntry.cpp
        AssetEntry.h
        AssetRef.cpp
        AssetRef.h)

enable_testing()

add_executable(
        my_test
        my_test.cpp
)
target_link_libraries(
        my_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(my_test)