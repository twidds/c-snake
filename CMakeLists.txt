cmake_minimum_required(VERSION 3.5)

#string(REPLACE "-O3" "-O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
#string(REPLACE "-O3" "-O1" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# if (MSVC)
#     add_compile_options(/W3 /WX)
# else()
#     add_compile_options(-W -Wall -Werror)
# endif()
#find_library(GDIP gdiplus)
#find_package(GDI REQUIRED)
#include_directories(${GDI_INCLUDE_DIRS})

project(snake)

#aux_source_directory(./src SRC_LIST)

add_executable(snake src/main.c src/graphics.c src/game.c)
target_link_libraries(snake gdiplus)