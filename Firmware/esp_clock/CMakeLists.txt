# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# add my own component directory to the list
list(APPEND EXTRA_COMPONENT_DIRS "./src")
list(APPEND EXTRA_COMPONENT_DIRS "../McuLib")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
add_compile_options(-include "${PROJECT_ROOT_DIR}/src/IncludeMcuLibConfig.h")
add_compile_options(-I "${PROJECT_ROOT_DIR}/src")

set(PROJECT_VER "0.1.0.1")
project(esp_clock)
