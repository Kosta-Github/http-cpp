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

#include "response.hpp"

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    struct HTTP_API responses {
        http::response add(http::response response);

        http::progress progress_all() const;

        void cancel_all();

        void wait_all();

        template<typename DURATION>
        inline auto wait_all_for(DURATION&& duration) -> decltype(std::future_status::ready) {
            auto timeout_time = std::chrono::system_clock::now() + duration;
            return wait_all_until(timeout_time);
        }

        template<typename TIMEOUT_TIME>
        inline auto wait_all_until(TIMEOUT_TIME&& timeout_time) -> decltype(std::future_status::ready) {
            for(auto&& r : m_responses) {
                if(r.data().wait_until(timeout_time) != std::future_status::ready) {
                    return std::future_status::timeout;
                }
            }
            return std::future_status::ready;
        }

    private:
        std::vector<http::response> m_responses;
    };

} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
