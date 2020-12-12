cmake_minimum_required(VERSION 3.16)
project(Thinkora)
enable_testing()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(extern)
add_subdirectory(src)

add_custom_target("patterns" ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/language" language
)
