#pragma once

#include "message.hpp"
#include "operation.hpp"
#include "progress_info.hpp"

#include <future>

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {

    typedef std::string request;

    struct HTTP_API response {
        std::future<http::message>& data();

        http::operation operation();
        http::request request();

        http::progress_info progress();

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
        std::shared_ptr<impl> m_impl;
    };

} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
