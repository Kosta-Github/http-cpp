#pragma once

#include "http-cpp.hpp"

namespace http {

    typedef int error_code;
    static const int HTTP_REQUEST_OK       = 0;
    static const int HTTP_REQUEST_CANCELED = 3001;
    HTTP_API bool error_code_is_known(http::error_code code);
    HTTP_API const char* error_code_to_string(http::error_code code);

} // namespace http
