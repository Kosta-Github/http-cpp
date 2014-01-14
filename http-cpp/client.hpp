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

#include "request.hpp"

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    typedef std::map<std::string, http::buffer> post_data;

    struct HTTP_API client {

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
        /// manner. The running request can also be canceled.
        http::request request(
            http::url       url,
            http::operation op          = http::HTTP_GET,
            http::headers   req_headers = http::headers(),
            http::buffer    put_data    = http::buffer(),
            http::post_data post_data   = http::post_data()
        );

        /// This call is similiar to the one above but with
        /// the added feature of registering a "continuation
        /// callback" which gets called back as soon as the
        /// request has finished (either successfully or with
        /// an error). The "continuation" gets the request
        /// object passed in as an argument to the callback.
        /// The callback will be called from within the
        /// context of another thread; and the work done
        /// within that callback should be very minimal since
        /// otherwise the sending and receiving of data gets
        /// blocked for too long.
        http::request request(
            std::function<void(http::request req)> continuationWith,
            http::url       url,
            http::operation op          = http::HTTP_GET,
            http::headers   req_headers = http::headers(),
            http::buffer    put_data    = http::buffer(),
            http::post_data post_data   = http::post_data()
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
            std::function<bool(http::message data, http::progress progress)> receive_cb,
            http::url       url,
            http::operation op          = http::HTTP_GET,
            http::headers   req_headers = http::headers(),
            http::buffer    put_data    = http::buffer(),
            http::post_data post_data   = http::post_data()
        );

        static void wait_for_all();
        static void cancel_all();

    public:
        client();
        client(client&& o) HTTP_CPP_NOEXCEPT;
        client& operator=(client&& o) HTTP_CPP_NOEXCEPT;
        ~client();
        
    private:
        client(client const&); // = delete;
        client& operator=(client const&); // = delete
    };
    
} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
