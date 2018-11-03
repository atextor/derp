cmake_minimum_required (VERSION 3.0)
file (GLOB PluginSources *.c)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
include_directories (${GLIB2_INCLUDE_DIRS} "../../${INCLUDE_DIR}")
link_directories (${GLIB2_LIBRARY_DIRS})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "../../${BIN_DIR}")

add_library (runexternal SHARED ${PluginSources})
target_link_libraries (runexternal ${GLIB2_LIBRARIES})
