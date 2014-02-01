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

#pragma once

#include "http-cpp.hpp"

#include <string>

namespace http {

    // an error_code <= 0 indicates a "no error situation"
    // a value > 0 should be treated as "an error occurred"
    typedef int error_code;
    static const error_code HTTP_REQUEST_FINISHED =    0;
    static const error_code HTTP_REQUEST_PROGRESS =   -1;
    static const error_code HTTP_REQUEST_CANCELED = 3002;

    HTTP_API bool error_code_is_known(http::error_code code);

    HTTP_API std::string error_code_to_string(http::error_code code);

} // namespace http
