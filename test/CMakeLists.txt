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
    
    add_test(
        NAME http_unittests
        COMMAND http_unittests
    )

endif()

set(
    SRC_MAIN_DRIVER_FILES
    main.cpp
    start_node_server.cpp
    start_node_server.hpp
)

set(
    SRC_UNIT_TEST_FILES
    client_unittests.cpp
    encode_unittests.cpp
    oauth1_unittests.cpp
)

set(S3_TEST_URI "" CACHE STRING "Point this variable to a protected s3 test url.")
if(S3_TEST_URI)
    set(SRC_UNIT_TEST_FILES ${SRC_UNIT_TEST_FILES} aws_s3_unittests.cpp)
    add_definitions(
        -DS3_TEST_URI="${S3_TEST_URI}"
    )
endif()

set(
    SRC_NODE_JS_FILES
    server/index.js
    server/request_handlers.js
    server/router.js
    server/server.js
)

add_executable(
    http_unittests
    ${SRC_MAIN_DRIVER_FILES}
    ${SRC_UNIT_TEST_FILES}
    ${SRC_NODE_JS_FILES}
)

source_group(main      FILES ${SRC_MAIN_DRIVER_FILES})
source_group(unittests FILES ${SRC_UNIT_TEST_FILES})
source_group(node.js   FILES ${SRC_NODE_JS_FILES})

target_link_libraries(
    http_unittests
    http-cpp
)
