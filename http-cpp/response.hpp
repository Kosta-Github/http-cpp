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
#include "operation.hpp"
#include "progress_info.hpp"

#include <future>

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    typedef std::string request;

    struct HTTP_API response {
        std::future<http::message>& data();

        http::operation operation();
        http::request request();

        http::progress_info progress();

        void cancel();

    public:
        response();
        response(response&& o) HTTP_CPP_NOEXCEPT;
        response& operator=(response&& o) HTTP_CPP_NOEXCEPT;
        ~response();

    private:
        response(response const&); // = delete;
        response& operator=(response const&); // = delete;

    private:
        friend struct client;
        struct impl;
        std::shared_ptr<impl> m_impl;
    };

} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
