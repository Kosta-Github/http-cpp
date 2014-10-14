cmake_minimum_required(VERSION 2.8)

project(http CXX)
message("configure: http-cpp library")

if(MSVC)
    set(CURL_HOME ${HTTP_CPP_3RD_PARTY_DIR}/curl)
    set(CURL_INCLUDE_DIRS "${CURL_HOME}/include")
    set(CURL_LIBRARIES_DEBUG "${CURL_HOME}/lib/libcurl_a_debug.lib")
    set(CURL_LIBRARIES_RELEASE "${CURL_HOME}/lib/libcurl_a.lib")
    add_definitions(-DCURL_STATICLIB)
    message("-- using curl lib: ${CURL_HOME}")
else(MSVC)
    find_package(CURL)
    message("-- using curl includes: ${CURL_INCLUDE_DIRS}")
    message("-- using curl libs:     ${CURL_LIBRARIES}")
    message("-- using curl version:  ${CURL_VERSION_STRING}")
endif(MSVC)

include_directories(
    ${CURL_INCLUDE_DIRS}
)

set(
    SRC_HTTP_FILES
    client.cpp
    client.hpp
    error_code.cpp
    error_code.hpp
    form_data.hpp
    http-cpp.hpp
    message.hpp
    operation.hpp
    progress.hpp
    status.cpp
    status.hpp
    request.hpp
    requests.cpp
    requests.hpp
    utils.cpp
    utils.hpp
)

set(
    SRC_HTTP_IMPL_FILES
    impl/curl_easy_wrap.hpp
    impl/curl_global_init_wrap.hpp
    impl/curl_multi_wrap.hpp
    impl/curl_share_wrap.hpp
)

set(
    SRC_HTTP_OAUTH1_FILES
    oauth1/client_oauth1.cpp
    oauth1/client.hpp
    oauth1/utils_oauth1.cpp
    oauth1/utils.hpp
)

set(
    SRC_HTTP_OAUTH1_BASE64_FILES
    ../3rdParty/base64/base64.cpp
    ../3rdParty/base64/base64.h
)

set(
    SRC_HTTP_OAUTH1_SHA1_FILES
    ../3rdParty/HMAC_SHA1/HMAC_SHA1.cpp
    ../3rdParty/HMAC_SHA1/HMAC_SHA1.h
    ../3rdParty/HMAC_SHA1/SHA1.cpp
    ../3rdParty/HMAC_SHA1/SHA1.h
)

add_library(
    http-cpp SHARED
    ${SRC_HTTP_FILES}
    ${SRC_HTTP_IMPL_FILES}
    ${SRC_HTTP_OAUTH1_FILES}
    ${SRC_HTTP_OAUTH1_BASE64_FILES}
    ${SRC_HTTP_OAUTH1_SHA1_FILES}
)

source_group(http                    FILES ${SRC_HTTP_FILES})
source_group(http\\impl              FILES ${SRC_HTTP_IMPL_FILES})
source_group(http\\oauth1            FILES ${SRC_HTTP_OAUTH1_FILES})
source_group(http\\oauth1\\base64    FILES ${SRC_HTTP_OAUTH1_BASE64_FILES})
source_group(http\\oauth1\\HMAC_SHA1 FILES ${SRC_HTTP_OAUTH1_SHA1_FILES})

if(MSVC)
    target_link_libraries(
        http-cpp
        debug       ${CURL_LIBRARIES_DEBUG}
        optimized   ${CURL_LIBRARIES_RELEASE}
    )
else(MSVC)
    target_link_libraries(
        http-cpp
        ${CURL_LIBRARIES}
    )
endif(MSVC)
