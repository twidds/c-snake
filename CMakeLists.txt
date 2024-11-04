cmake_minimum_required(VERSION 3.5)

project(snake)

# Output binaries to bin/
add_compile_definitions(SUPPORT_FILEFORMAT_BMP)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(raylib/raylib-5.0)
add_subdirectory(src)



# include_directories(${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
# target_link_libraries(snake gdiplus)
configure_file(assets/snake_spritesheet.bmp bin/assets/snake_spritesheet.bmp COPYONLY)
configure_file(assets/food_spritesheet.bmp bin/assets/food_spritesheet.bmp COPYONLY)
configure_file(assets/backgrounds_spritesheet.bmp bin/assets/backgrounds_spritesheet.bmp COPYONLY)