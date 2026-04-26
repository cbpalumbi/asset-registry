cmake_minimum_required(VERSION 3.25)
project(AssetRegistryLib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(AssetRegistryLib
        src/AssetEntry.cpp
        src/AssetRef.cpp
        src/Registry.cpp
)

target_include_directories(AssetRegistryLib PUBLIC include)