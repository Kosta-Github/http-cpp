#pragma once

#include "http-cpp.hpp"

namespace http {

    // an error_code <= 0 indicates a "no error situation"
    // a value > 0 should be treated as "an error occurred"
    typedef int error_code;
    static const error_code HTTP_REQUEST_FINISHED =    0;
    static const error_code HTTP_REQUEST_PROGRESS =   -1;
    static const error_code HTTP_REQUEST_CANCELED = 3002;

    HTTP_API bool error_code_is_known(http::error_code code);

    HTTP_API const char* error_code_to_string(http::error_code code);

} // namespace http
