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

#include "request.hpp"

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    /// This helper class 'requests' can be used to track a set of HTTP requests
    /// in order to treat them as group. This allows to cancel all tracked ...
    struct HTTP_API requests {

        /// Forward this call 'request()' call to the given 'client' object,
        /// add the created 'request' object to this list of tracked requests,
        /// and return it also back to the caller.
        http::request request(
            http::client&   client,
            http::url       url,
            http::operation op = http::HTTP_GET
        );

        /// Adds the given 'req' object to the list of tracked requests.
        http::request add(http::request req);

        /// Returns the current progress information for all tracked requests.
        http::progress progress_all() const;

        /// Cancels all tracked requests.
        void cancel_all();

        /// Waits for all tracked requests to finish.
        void wait_all();

        /// Waits for all tracked requests to finish using the given timeout duration.
        template<typename DURATION>
        inline auto wait_for_all(DURATION&& duration) -> decltype(std::future_status::ready) {
            return wait_until_all(std::chrono::system_clock::now() + duration);
        }

        /// Waits for all tracked requests to finish using the given absolute timeout time.
        template<typename TIMEOUT_TIME>
        inline auto wait_until_all(TIMEOUT_TIME&& timeout_time) -> decltype(std::future_status::ready) {
            for(auto&& r : reqs) {
                auto status = r.wait_until(timeout_time);
                if(status != std::future_status::ready) {
                    return status;
                }
            }
            return std::future_status::ready;
        }

        /// Provide direct access to the list of tracked requests.
        std::vector<http::request> reqs;
    };

} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
