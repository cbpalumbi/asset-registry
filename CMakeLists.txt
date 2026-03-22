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

add_executable(AssetRegistry main.cpp
        Registry.cpp
        Registry.h
        AssetEntry.cpp
        AssetEntry.h
        AssetRef.cpp
        AssetRef.h)