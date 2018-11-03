cmake_minimum_required (VERSION 3.0)

# Project configuration
project (Derp)
set (BIN_DIR "bin")
set (INCLUDE_DIR "include")
set (CLIPS_DIR "clips")
set (PLUGIN_DIR "plugins")

# Configure sources and libraries
add_subdirectory ("${CLIPS_DIR}")
file (GLOB DerpSources *.c)
include_directories ("${INCLUDE_DIR}" "${CLIPS_DIR}")
find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
include_directories (${GLIB2_INCLUDE_DIRS})
link_directories (${GLIB2_LIBRARY_DIRS} "${CLIPS_DIR}")
add_definitions (${GLIB2_CFLAGS_OTHER})
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

# Target directory
make_directory ("bin")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}")

# Linking
add_executable (derp ${DerpSources})
target_link_libraries (derp clips m ${GLIB2_LIBRARIES} pthread dl)

# Plugins
add_subdirectory ("${PLUGIN_DIR}/plugin1")
add_subdirectory ("${PLUGIN_DIR}/raptor")
add_subdirectory ("${PLUGIN_DIR}/runexternal")

# Resources
configure_file ("dcterms.rdf" "${BIN_DIR}")
