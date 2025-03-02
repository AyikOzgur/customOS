cmake_minimum_required(VERSION 3.13)

################################################################################
## EXECUTABLE-PROJECT
## name and version
################################################################################
project(receiveApp LANGUAGES C)

################################################################################
## SETTINGS
## basic project settings before use
################################################################################
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Define the cross-compiler toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)


# Creating output directory architecture in accordance with GNU guidelines
set(BINARY_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}/lib")

################################################################################
## TARGET
## create target and add include path
################################################################################
# Create glob files for *.h, *.c
file (GLOB H_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file (GLOB C_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Concatenate the results (glob files) to variable
set(SOURCES ${C_FILES} ${H_FILES})

if (NOT TARGET ${PROJECT_NAME})
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()
