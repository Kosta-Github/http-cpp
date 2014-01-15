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

#include "message.hpp"

#include <cassert>

namespace http {

    struct post_content {
        inline post_content(std::string n, http::buffer c, std::string t = "") :
            name(std::move(n)), content(std::move(c)), type(std::move(t))
        {
            assert(!name.empty());
        }

        std::string     name;
        http::buffer    content;
        std::string     type;

#if defined(HTTP_CPP_NEED_EXPLICIT_MOVE)
        post_content(post_content&& o) HTTP_CPP_NOEXCEPT { operator=(std::move(o)); }
        post_content& operator=(post_content&& o) HTTP_CPP_NOEXCEPT {
            if(this != &o) {
                name    = std::move(o.name);
                content = std::move(o.content);
                type    = std::move(o.type);
            }
            return *this;
        }
#endif // defined(HTTP_CPP_NEED_EXPLICIT_MOVE)
    };

    typedef std::vector<post_content> post_data;

} // namespace http
