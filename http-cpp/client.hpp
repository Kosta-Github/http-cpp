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

#include "./form_data.hpp"
#include "./request.hpp"

#include <cassert>

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    struct HTTP_API client {
        virtual ~client() { }

        /// Constructs a new client object and initializes it with
        /// appropriate default values.
        client();

        /// These headers will be added to each request started
        /// from this client.
        http::headers headers;

        /// If receive_file is specified a file with the given filename
        /// will be opened for writing and the received content will be
        /// written to it. Note that the `body` member of the request
        /// object will not be filled with the received content in this
        /// case.
        /// If the file could not be opened for writing an exception will be
        /// thrown from the request() method.
        std::string receive_file;

        /// This data buffer will be used for sending data from the client
        /// to the HTTP server. Note that you should not send data with
        /// some specific HTTP request methods such as GET or DELETE.
        /// In order to avoid multi-threading issues, object lifetime
        /// issues, or excessive memory copies, this buffer will be
        /// move into a private area once the request gets started.
        http::buffer send_data;

        /// If send_file is specified the content of the referenced
        /// file is used for sending from the client to the server.
        /// See also the description for the send_data member.
        /// If the file could not be opened for reading an exception
        /// will be thrown from the request() method.
        std::string send_file;

        /// This post_data will be send for a POST request from the client
        /// to the server. In order to avoid multi-threading issues, object
        /// lifetime issues, or excessive memory copies, this data will
        /// be move into a private area once the request gets started.
        http::form_data post_form;

        /// Specifies the maximum time in seconds that a connect is
        /// allowed to get established. Default value is 300 seconds.
        size_t connect_timeout;

        /// Specifies the maximum time in seconds that a whole request
        /// can run before it times out. Set to 0 if no timeout should
        /// be used (default).
        size_t request_timeout;

        /// Specifies that the returned result from the server might be
        /// accepted as compressed by method libcurl understands and
        /// libcurl will do the decompression transparently, which means
        /// the result body will contain the already decompressed data.
        /// The default value is true for this flag in order to gain
        /// some network bandwidth if possible.
        bool accept_compressed;

        /// If an on_finish callback is provided the callback
        /// will be called immediately after finishing the
        /// request; the provided request object can then be
        /// inspected whether the request finished successfully or
        /// with a failure; the callback will be called from the
        /// context of another thread so you need to ensure that
        /// no multi-threading issues can occurr within the
        /// callback implementation and the callback should
        /// return as fast as possible since it will block
        /// sending and receiving further data for other requests
        /// running in parallel. The on_progress member will be
        /// clear once a request gets started.
        std::function<void(http::request)> on_finish;

        /// If an on_receive callback is provided the callback
        /// will be called each time new data has been received;
        /// the callback will be called from the context of another
        /// thread so you need to ensure that no multi-threading
        /// issues can occurr within the callback implementation
        /// and the callback should return as fast as possible since
        /// it will block sending and receiving further data for other
        /// requests running in parallel. The boolean return value of
        /// the callback indicate whether the running request should
        /// be aborted (false) or should go on (true). Note, that the
        /// storage of the received data packets is the responsibility
        /// of the on_receive callback and that it will not be added to
        /// the data member of the returned request object. The on_receive
        /// member will be clear once a request gets started.
        std::function<bool(http::message data, http::progress progress)> on_receive;

        /// If an on_progress callback is provided the callback
        /// will be called periodically with the current
        /// progress info; returning "false" from the progress
        /// callback will cancel the corresponding request.
        /// The on_progress member will be clear once a request
        /// gets started.
        std::function<bool(http::progress)> on_progress;

        /// If an on_debug callback is provided the callback
        /// will be called with debugging information passed
        /// through directly from libcurl; the callback will be
        /// called from the context of another thread so you
        /// need to ensure that no multi-threading issues can
        /// occurr within the callback implementation and the
        /// callback should return as fast as possible since it
        /// will block sending and receiving further data for
        /// other requests running in parallel. The on_debug
        /// member will be clear once a request gets started.
        std::function<void(std::string const&)> on_debug;

        /// Starts the request and returns immediately.
        /// The result can be polled from the message-future
        /// object contained in the returned request object.
        /// The request object can also be queried about the
        /// actual progress and upload/download speed while
        /// the request is still running in a non-blocking
        /// manner. The running request can be canceled.
        virtual http::request request(
            http::url       url,
            http::operation op = http::OP_GET()
        );

        static void wait_for_all();
        static void cancel_all();
    };
    
} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
