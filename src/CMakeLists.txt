add_executable(snake main.c menu_screen.c)
target_include_directories(snake PRIVATE ${CMAKE_SOURCE_DIR}/raylib/raylib-5.0/src)
target_link_libraries(snake raylib)