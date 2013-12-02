cmake_minimum_required(VERSION 2.8)

project(http-cpp CXX)
message("configure: http-cpp")

# Check for cxx0x, cxx11, and libcxx11 options
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" HAS_CXX0x)
CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++11 -stdlib=libc++" HAS_LIBCXX11)

# switch to the right compiler
if(HAS_LIBCXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
elseif(HAS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(HAS_CXX0x)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(MSVC)
#    add_definitions()
endif()

set(HTTP_CPP_3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty)

include_directories(.)

# configure unit tests
enable_testing()

add_subdirectory(http-cpp)
add_subdirectory(test)
