#pragma once

#include "http-cpp.hpp"

#include <string>

namespace http {

    HTTP_API std::string escape(std::string s);
    HTTP_API std::string unescape(std::string s);

} // namespace http
