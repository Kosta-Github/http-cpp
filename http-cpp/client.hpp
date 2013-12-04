#pragma once

#include "http-cpp.hpp"
#include "operation.hpp"
#include "status.hpp"

#include <functional>
#include <future>
#include <map>
#include <string>
#include <vector>

namespace http {

    struct response;
    struct client;

    typedef std::string request;
    typedef std::vector<char> buffer;
    typedef std::map<std::string, std::string> headers;


    enum error_code {
        HTTP_REQUEST_OK,
        HTTP_REQUEST_CANCELED,
        HTTP_REQUEST_ERROR
    };

    struct HTTP_API response {
        struct info {
            http::error_code    error_code;
            std::string         error_string;
            http::status        status;
            http::headers       headers;
            http::buffer        body;
        };
        std::future<info>& data();

        void progress(size_t& outDownCur, size_t& outDownTotal, size_t& outUpCur, size_t& outUpTotal);

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
        impl* m_impl;
    };

    struct HTTP_API client {
        typedef std::function<bool(http::buffer data, http::status status)> receive_body_cb;

        http::response request(
            http::operation op,
            http::request req,
            receive_body_cb receive_cb = nullptr,
            http::buffer send_body = http::buffer(),
            std::string send_content_type = "x-application/octet-stream"
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
