#pragma once

#include "response.hpp"

namespace http {

    struct HTTP_API client {

        /// Starts the request and returns immediately.
        /// The result can be polled from the message-future
        /// object contained in the response object. The
        /// response object can also be queried about the
        /// actual progress and upload/download speed while
        /// the request is still running in a non-blocking
        /// manner. The running request can be canceled via the
        /// response object as well.
        http::response request(
            http::request   req,
            http::operation op                  = http::HTTP_GET,
            http::headers   headers             = http::headers(),
            http::buffer    send_data           = http::buffer(),
            std::string     data_content_type   = "application/octet-stream"
        );

        /// This call is similiar to the one above but with
        /// the added service of registering a "continuation
        /// callback" which gets called back as soon as the
        /// request has finished (either successfully or with
        /// an error). The "continuation" gets the respsonse
        /// object passed in as an argument to the callback.
        /// The callback will happening from within the
        /// context of another thread; and the work done
        /// within that callback should be very minimal since
        /// otherwise the sending and receiving of data gets
        /// blocked for too long.
        http::response request(
            std::function<void(http::response response)> continuationWith,
            http::request   req,
            http::operation op                  = http::HTTP_GET,
            http::headers   headers             = http::headers(),
            http::buffer    send_data           = http::buffer(),
            std::string     data_content_type   = "application/octet-stream"
        );

        /// This call is similiar to the one above but with
        /// the added service of registering a "progress
        /// callback" which gets called back as soon as new
        /// data has been received. The newly received data
        /// together with some progress infos about the running
        /// request are passed as arguments to the callback.
        /// The return value of the callback indicates whether
        /// the request should still be handled (true) or
        /// canceled (false). Note that callback can also be
        /// called without newly received data but with only
        /// a progress update. Therefore, the error_code within
        /// the passed-in message object needs to be evaluated
        /// and checked for HTTP_REQUEST_FINISHED,
        /// HTTP_REQUEST_PROGRESS, HTTP_REQUEST_CANCELED, and
        /// all other positive values indicating an error.
        void request_stream(
            std::function<bool(http::message data, http::progress_info progress)> receive_cb,
            http::request   req,
            http::operation op                  = http::HTTP_GET,
            http::headers   headers             = http::headers(),
            http::buffer    send_data           = http::buffer(),
            std::string     data_content_type   = "application/octet-stream"
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

    private:
        struct impl;
        impl* m_impl;
    };
    
} // namespace http
