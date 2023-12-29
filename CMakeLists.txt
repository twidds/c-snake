cmake_minimum_required(VERSION 3.5)

if (MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-W -Wall -Werror)
endif()

project(snake)

#aux_source_directory(./src SRC_LIST)

add_executable(snake src/main.c)