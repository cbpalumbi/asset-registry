cmake_minimum_required(VERSION 3.25)
include(FetchContent)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

add_executable(AssetRegistryGame
        main.cpp
        TextureCache.h
        Player.cpp
        Player.h
        World.cpp
        World.h
)

target_link_libraries(AssetRegistryGame PRIVATE AssetRegistryLib raylib)