cmake_minimum_required(VERSION 2.8)

project(http CXX)
message("configure: http-cpp unittests")

# configure catch
include_directories("${HTTP_CPP_3RD_PARTY_DIR}/catch/include")
add_definitions(-DCATCH_CONFIG_PREFIX_ALL)

find_program(
	python
	NAMES python
)

if(python STREQUAL "python-NOTFOUND")
	message("-- no 'python' found: skipping http request unit tests against a dummy server...")
else()
	message("-- 'python' found: ${python}")
	message("-- configuring http request unit tests against a dummy server")
	
    add_definitions(-DPYTHON_EXE="${python}")
endif()

add_executable(
	http-cpp_unittests
	main.cpp
	http-cpp_unittests.cpp
)
	 
target_link_libraries(
	http-cpp_unittests
	http-cpp
)

add_test(
	NAME http-cpp_unittests
	COMMAND http-cpp_unittests
)
