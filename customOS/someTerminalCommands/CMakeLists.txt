cmake_minimum_required(VERSION 3.13)

################################################################################
## PROJECT
################################################################################
project(AllTerminalCommands LANGUAGES C)

################################################################################
## SETTINGS
################################################################################
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set output directories
set(BINARY_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}/lib")

################################################################################
## ADD ALL DIRECTORIES (EXCEPT 'build')
################################################################################
# Get all directories inside the project root
file(GLOB CHILDREN RELATIVE ${CMAKE_SOURCE_DIR} *)
foreach (child ${CHILDREN})
    if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${child} AND NOT child STREQUAL "build")
        add_subdirectory(${child})
    endif()
endforeach()