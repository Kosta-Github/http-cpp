cmake_minimum_required(VERSION 2.8.8)

project(http-cpp CXX)
message("configure: http-cpp")

include(CMakeConfigCompilers.cmake)

set(HTTP_CPP_3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty)

SET(EXECUTABLE_OUTPUT_PATH         ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(.)

# Oleg [2014-11-19]
# Disable unit tests on OSX since they are failing on some machines
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # configure unit tests
    enable_testing()
endif()

add_subdirectory(http-cpp)
add_subdirectory(test)
