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

#include "message.hpp"
#include "operation.hpp"
#include "progress.hpp"
#include "utils.hpp"

#include <future>

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    typedef std::string url;

    struct HTTP_API request {
        std::shared_future<http::message>& data();

        http::url url() const;
        http::operation operation() const;

        http::progress progress() const;

        void cancel();

        inline void wait() { http::wait(data()); }

        template<typename TIME>
        inline auto wait_for(TIME&& t) -> decltype(std::future_status::ready) {
            return http::wait_for(data(), std::forward<TIME>(t));
        }

        template<typename TIME>
        inline auto wait_until(TIME&& t) -> decltype(std::future_status::ready) {
            return http::wait_until(data(), std::forward<TIME>(t));
        }

    private:
        friend struct client;
        struct impl;
        std::shared_ptr<impl> m_impl;
    };

} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
