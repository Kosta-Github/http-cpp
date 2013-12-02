cmake_minimum_required(VERSION 2.8)

project(http CXX)
message("configure: http-cpp unittests")

include_directories(
	"${HTTP_CPP_3RD_PARTY_DIR}/catch/include"
)

add_definitions(
	-DCATCH_CONFIG_PREFIX_ALL
)

find_program(
	nodejs
	NAMES node
	PATHS "${HTTP_CPP_3RD_PARTY_DIR}/node.js/Win64"
)

if(nodejs STREQUAL "nodejs-NOTFOUND")
	message("-- no node.js found: skipping unit tests...")
else()
	message("-- node.js found: ${nodejs}")
	message("-- configuring http unit tests")
	
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
endif()
