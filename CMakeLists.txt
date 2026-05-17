cmake_minimum_required(VERSION 3.5)

project(snake)

# Output binaries to bin/
add_compile_definitions(SUPPORT_FILEFORMAT_BMP)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(raylib/raylib-5.0)
#add_subdirectory(src)
#add_subdirectory(images)

#PRE-BUILD: Build generator for image headers
add_executable(generator img_builder/main.c)
target_include_directories(generator PRIVATE ${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
target_link_libraries(generator raylib)

set(SNAKE_IMG_ASSET ${CMAKE_SOURCE_DIR}/assets/snake_spritesheet.bmp)
set(SNAKE_IMG_HEADER ${CMAKE_SOURCE_DIR}/src/images/snake_spritesheet.h)

set(BACKGROUNDS_IMG_ASSET ${CMAKE_SOURCE_DIR}/assets/backgrounds_spritesheet.bmp)
set(BACKGROUNDS_IMG_HEADER ${CMAKE_SOURCE_DIR}/src/images/backgrounds_spritesheet.h)

set(FOOD_IMG_ASSET ${CMAKE_SOURCE_DIR}/assets/food_spritesheet.bmp)
set(FOOD_IMG_HEADER ${CMAKE_SOURCE_DIR}/src/images/food_spritesheet.h)


#PRE-BUILD: Generate header files with image data
add_custom_target(
    pre_build_snakeimg
    COMMAND generator ${SNAKE_IMG_ASSET} ${SNAKE_IMG_HEADER}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "run generator for snake image header"
    SOURCES img_builder/main.c
)

add_custom_target(
    pre_build_foodimg
    COMMAND generator ${FOOD_IMG_ASSET} ${FOOD_IMG_HEADER}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "run generator for food image header"
    SOURCES img_builder/main.c
)

add_custom_target(
    pre_build_bgimg
    COMMAND generator ${BACKGROUNDS_IMG_ASSET} ${BACKGROUNDS_IMG_HEADER}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "run generator for backgrounds image header"
    SOURCES img_builder/main.c
)

#BUILD: Snake game
add_executable(snake src/main.c src/menu_screen.c src/gui.c src/arena.c src/game_screen.c src/images.c)
target_include_directories(snake PRIVATE ${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
target_link_libraries(snake raylib)

add_dependencies(snake pre_build_snakeimg pre_build_foodimg pre_build_bgimg)



# include_directories(${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
# target_link_libraries(snake gdiplus)
# configure_file(assets/snake_spritesheet.bmp bin/assets/snake_spritesheet.bmp COPYONLY)
# configure_file(assets/food_spritesheet.bmp bin/assets/food_spritesheet.bmp COPYONLY)
# configure_file(assets/backgrounds_spritesheet.bmp bin/assets/backgrounds_spritesheet.bmp COPYONLY)