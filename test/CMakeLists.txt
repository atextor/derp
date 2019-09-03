cmake_minimum_required (VERSION 3.0)

project (DerpTest)
set (CUNIT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cunit")

# Checkout cunit as sub-build
include(ExternalProject)
ExternalProject_Add(cunit
  GIT_REPOSITORY    https://gitlab.com/cunity/cunit.git
  GIT_TAG           master
  SOURCE_DIR        "${CUNIT_DIR}"
  BINARY_DIR        "${CUNIT_DIR}"
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

# Configure sources and libraries
file (GLOB TestSources *.c)
include_directories (
  "${CUNIT_DIR}/CUnit"
  "${CUNIT_DIR}/CUnit/CUnit"
  "${CUNIT_DIR}/CUnit/CUnit/generated-headers"
)
link_directories ("${CUNIT_DIR}/CUnit")

add_executable (testsuite ${TestSources})
add_dependencies (testsuite cunit)
target_link_libraries (testsuite libcunit.a)

# Execute tests
add_custom_target (run_testsuite ALL
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/bin/testsuite"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Run test suite in ${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies (run_testsuite testsuite)

