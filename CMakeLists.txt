cmake_minimum_required(VERSION 3.5)

project(snake)

# Output binaries to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(raylib/raylib-5.0)
add_subdirectory(src)

# include_directories(${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
# target_link_libraries(snake gdiplus)
configure_file(assets/spritesheet_actual.bmp bin/spritesheet.bmp COPYONLY)