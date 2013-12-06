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
	find_package(curl REQUIRED)
endif(MSVC)

include_directories(
    ${CURL_INCLUDE_DIRS}
)

add_library(
	http-cpp SHARED
	client.cpp
	client.hpp
    error_code.cpp
    error_code.hpp
	http-cpp.hpp
    message.hpp
	operation.cpp
	operation.hpp
    progress_info.hpp
	status.cpp
	status.hpp
    response.hpp
    utils.hpp
    impl/curl_easy_wrap.hpp
    impl/curl_global_init_wrap.hpp
    impl/curl_multi_wrap.hpp
    impl/curl_share_wrap.hpp
)

if(MSVC)
	target_link_libraries(
		http-cpp
		debug   	${CURL_LIBRARIES_DEBUG}
		optimized	${CURL_LIBRARIES_RELEASE}
	)
else(MSVC)
	target_link_libraries(
		http-cpp
		${CURL_LIBRARIES}
	)
endif(MSVC)
