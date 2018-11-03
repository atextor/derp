cmake_minimum_required (VERSION 3.0)

file (GLOB ClipsSources *.c)
list (REMOVE_ITEM ClipsSources main.c)
add_compile_options (-fvisibility=hidden -Wall)
add_library (clips STATIC ${ClipsSources})
