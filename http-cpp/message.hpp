#pragma once

#include "error_code.hpp"
#include "status.hpp"

#include <map>
#include <string>
#include <vector>

namespace http {

    typedef std::vector<char> buffer;
    typedef std::map<std::string, std::string> headers;

    struct message {
        message(
            http::error_code ec = http::HTTP_REQUEST_PROGRESS,
            http::status     s  = http::HTTP_000_UNKNOWN,
            http::headers    h  = http::headers(),
            http::buffer     b  = http::buffer()
        ) :
            error_code(ec),
            status(s),
            headers(std::move(h)),
            body(std::move(b))
        { }

        http::error_code    error_code;
        http::status        status;
        http::headers       headers;
        http::buffer        body;
    };

} // namespace http
