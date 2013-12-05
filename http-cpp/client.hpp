#pragma once

#include "response.hpp"

namespace http {

    struct HTTP_API client {

        http::response request(
            http::operation op,
            http::request req,
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
        friend struct response;
        struct impl;
        impl* m_impl;
    };
    
} // namespace http
