# root CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(AssetRegistry)
set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/W4 /fsanitize=address /Zi)
    add_link_options(/DEBUG)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(game)