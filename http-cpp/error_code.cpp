//
// The MIT License (MIT)
//
// Copyright (c) 2013-2014 by Konstantin (Kosta) Baumann & Autodesk Inc.
//
// Permission is hereby granted, free of charge,  to any person obtaining a copy of
// this software and  associated documentation  files  (the "Software"), to deal in
// the  Software  without  restriction,  including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software,  and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this  permission notice  shall be included in all
// copies or substantial portions of the Software.
//
// THE  SOFTWARE  IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
// IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "error_code.hpp"

#include <curl/curl.h>

bool http::is_error(http::error_code code) {
    return (code > 0);
}

bool http::is_valid(http::error_code code) {
    return (
        ((CURLE_OK   <= code -    0) && (code -    0 < CURL_LAST))      ||
        ((CURLM_OK   <= code - 1000) && (code - 1000 < CURLM_LAST))     ||
        ((CURLSHE_OK <= code - 2000) && (code - 2000 < CURLSHE_LAST))   ||
        (code == HTTP_ERROR_REPORT_PROGRESS)                            ||
        (code == HTTP_ERROR_REQUEST_CANCELED)                           ||
        (code == HTTP_ERROR_COULDNT_OPEN_SEND_FILE)                     ||
        (code == HTTP_ERROR_COULDNT_OPEN_RECEIVE_FILE)
    );
}

std::string http::to_string(http::error_code code) {
    if((0    <= code) && (code < 1000)) {           return curl_easy_strerror( static_cast<CURLcode>(  code -    0)); }
    if((1000 <= code) && (code < 2000)) {           return curl_multi_strerror(static_cast<CURLMcode>( code - 1000)); }
    if((2000 <= code) && (code < 3000)) {           return curl_share_strerror(static_cast<CURLSHcode>(code - 2000)); }
    switch(code) {
        case HTTP_ERROR_REPORT_PROGRESS:            return "progress";
        case HTTP_ERROR_REQUEST_CANCELED:           return "request canceled";
        case HTTP_ERROR_COULDNT_OPEN_SEND_FILE:     return "could not open send file";
        case HTTP_ERROR_COULDNT_OPEN_RECEIVE_FILE:  return "could not open receive file";
        default:                                    return "unknown error code";
    }
}
