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

#include "post_data.hpp"
#include "request.hpp"

#include <cassert>

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    struct HTTP_API client {

        /// Constructs a new client object and initializes it with
        /// appropriate default values.
        client();

        /// These headers will be added to each request started
        /// from this client.
        http::headers headers;

        /// Specifies the maximum time in seconds that a connect is
        /// allowed to get established. Default value is 300 seconds.
        size_t connect_timeout;

        /// Specifies the maximum time in seconds that a whole request
        /// can run before it times out. Set to 0 if no timeout should
        /// be used (default).
        size_t request_timeout;

        /// Starts the request and returns immediately.
        /// The result can be polled from the message-future
        /// object contained in the returned request object.
        /// The request object can also be queried about the
        /// actual progress and upload/download speed while
        /// the request is still running in a non-blocking
        /// manner. The running request can be canceled.
        /// If an on_progress callback is provided the callback
        /// will be called periodically with the current
        /// progress info; returning "false" from the progress
        /// callback will cancel the corresponding request.
        /// If an on_finish callback is provided the callback
        /// will be called immediately after finishing the
        /// request; the provided request object can then be
        /// queried whether the request finished successfully or
        /// with a failure; the callback will be called from the
        /// context of another thread so you need to ensure that
        /// no multi-threading issues can ocurr within the
        /// callback implementation and the callback should
        /// return as fast as possible since it will block
        /// sending and receiving further data for other requests
        /// running in parallel.
        http::request request(
            http::url                           url,
            http::operation                     op          = http::HTTP_GET,
            std::function<bool(http::progress)> on_progress = nullptr,
            std::function<void(http::request)>  on_finish   = nullptr,
            http::headers                       req_headers = http::headers(),
            http::buffer                        put_data    = http::buffer(),
            http::post_data                     post_data   = http::post_data()
        );

        /// This call is similiar to the one above but with
        /// the added feature of registering a "progress
        /// callback" which gets called back periodically, e.g.,
        /// whenever new data has been received or to report
        /// some progress. The newly received data together
        /// with the information about the progress for the running
        /// request are passed as arguments to the callback.
        /// The return value of the callback indicates whether
        /// the request should still be handled (true) or
        /// canceled (false). Note that the callback can also be
        /// called without newly received data but with only
        /// a progress update. Therefore, the error_code within
        /// the passed-in message object needs to be evaluated
        /// and checked for HTTP_REQUEST_FINISHED,
        /// HTTP_REQUEST_PROGRESS, HTTP_REQUEST_CANCELED; all
        /// other positive values indicating an error.
        void request_stream(
            std::function<bool(http::message data, http::progress progress)> on_receive,
            http::url       url,
            http::operation op          = http::HTTP_GET,
            http::headers   req_headers = http::headers(),
            http::buffer    put_data    = http::buffer(),
            http::post_data post_data   = http::post_data()
        );

        static void wait_for_all();
        static void cancel_all();
    };
    
} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
