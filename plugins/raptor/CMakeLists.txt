cmake_minimum_required (VERSION 3.0)
file (GLOB PluginSources *.c)

find_package (PkgConfig REQUIRED)
pkg_check_modules (RAPTOR2 REQUIRED raptor2)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
include_directories (${RAPTOR2_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS} "../../${INCLUDE_DIR}")
link_directories (${RAPTOR2_LIBRARY_DIRS} ${GLIB2_LIBRARY_DIRS})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "../../${BIN_DIR}")
add_compile_options (-std=c99 -Wall -Werror)

add_library (raptor SHARED ${PluginSources})
target_link_libraries (raptor ${RAPTOR2_LIBRARIES} ${GLIB2_LIBRARIES})
