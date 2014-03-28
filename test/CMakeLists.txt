cmake_minimum_required(VERSION 2.8)

project(http-cpp CXX)
message("configure: http-cpp unittests")

# configure cute
include_directories("${HTTP_CPP_3RD_PARTY_DIR}/cute/")

find_program(
	node
	NAMES node
	PATHS ${HTTP_CPP_3RD_PARTY_DIR}/node.js/Win64
)

if(node STREQUAL "node-NOTFOUND")
	message("-- no 'node' found: skipping http request unit tests against a dummy server...")
else()
	message("-- 'node' found: ${node}")
	message("-- configuring http request unit tests against a dummy server")
	
	add_definitions(
		-DNODE_EXE="${node}"
		-DNODE_SERVER_JS="${CMAKE_CURRENT_SOURCE_DIR}/server/index.js"
	)
endif()

add_executable(
	http_unittests
	main.cpp
	http_unittests.cpp
	start_node_server.cpp
	start_node_server.hpp
	server/index.js
	server/request_handlers.js
	server/router.js
	server/server.js
)
	 
target_link_libraries(
	http_unittests
	http-cpp
)

add_test(
	NAME http_unittests
	COMMAND http_unittests
)
