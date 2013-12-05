#pragma once

#include "response.hpp"

namespace http {

    struct HTTP_API client {

        http::response request(
            http::request req,
            http::operation op = http::HTTP_GET,
            http::headers headers = http::headers(),
            http::buffer send_data = http::buffer(),
            std::string data_content_type = "application/octet-stream"
        );

        void request_callback(
            std::function<void(http::response response)> receive_cb,
            http::request req,
            http::operation op = http::HTTP_GET,
            http::headers headers = http::headers(),
            http::buffer send_data = http::buffer(),
            std::string data_content_type = "application/octet-stream"
        );
        
        void request_stream(
            std::function<bool(http::message data, http::progress_info progress)> receive_cb,
            http::request req,
            http::operation op = http::HTTP_GET,
            http::headers headers = http::headers(),
            http::buffer send_data = http::buffer(),
            std::string data_content_type = "application/octet-stream"
        );

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
