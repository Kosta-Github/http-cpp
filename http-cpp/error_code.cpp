#include "error_code.hpp"

#include <curl/curl.h>

bool http::error_code_is_known(http::error_code code) {
    return (
        ((CURLE_OK   <= code -    0) && (code -    0 < CURL_LAST))  ||
        ((CURLM_OK   <= code - 1000) && (code - 1000 < CURLM_LAST)) ||
        ((CURLSHE_OK <= code - 2000) && (code - 2000 < CURLSHE_LAST)) ||
        (code == HTTP_REQUEST_CANCELED)
    );
}

const char* http::error_code_to_string(http::error_code code) {
    if((0    <= code) && (code < 1000)) { return curl_easy_strerror( static_cast<CURLcode>(  code -    0)); }
    if((1000 <= code) && (code < 2000)) { return curl_multi_strerror(static_cast<CURLMcode>( code - 1000)); }
    if((2000 <= code) && (code < 3000)) { return curl_share_strerror(static_cast<CURLSHcode>(code - 2000)); }
    if(code == HTTP_REQUEST_CANCELED) { return "request canceled"; }
    return "unknown error code";
}
