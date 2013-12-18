//
// The MIT License (MIT)
//
// Copyright (c) 2013 by Konstantin (Kosta) Baumann & Autodesk Inc.
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

#include "error_code.hpp"
#include "status.hpp"

#include <map>
#include <string>
#include <vector>

namespace http {

    typedef std::vector<char> buffer;
    typedef std::map<std::string, std::string> headers;

    struct message {
        message(
            http::error_code ec = http::HTTP_REQUEST_PROGRESS,
            http::status     s  = http::HTTP_000_UNKNOWN,
            http::headers    h  = http::headers(),
            http::buffer     b  = http::buffer()
        ) :
            error_code(ec),
            status(s),
            headers(std::move(h)),
            body(std::move(b))
        { }

        http::error_code    error_code;
        http::status        status;
        http::headers       headers;
        http::buffer        body;

#if defined(HTTP_CPP_NEED_EXPLICIT_MOVE)
        message(message&& o) HTTP_CPP_NOEXCEPT { operator=(std::move(o)); }
        message& operator=(message&& o) HTTP_CPP_NOEXCEPT {
            if(this != &o) {
                error_code = std::move(o.error_code);
                status = std::move(o.status);
                headers = std::move(o.headers);
                body = std::move(o.body);
            }
            return *this;
        }
#endif // defined(HTTP_CPP_NEED_EXPLICIT_MOVE)
    };

} // namespace http
