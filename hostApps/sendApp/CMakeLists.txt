cmake_minimum_required(VERSION 3.13)



################################################################################
## EXECUTABLE-PROJECT
## name and version
################################################################################
project(sendApp LANGUAGES C)



################################################################################
## SETTINGS
## basic project settings before use
################################################################################
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# creating output directory architecture in accordance with GNU guidelines
set(BINARY_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARY_DIR}/lib")



################################################################################
## TARGET
## create target and add include path
################################################################################
# create glob files for *.h, *.c
file (GLOB H_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file (GLOB CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
# concatenate the results (glob files) to variable
set  (SOURCES ${CPP_FILES} ${H_FILES})
if (NOT TARGET ${PROJECT_NAME})
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()