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

#include "./http-cpp.hpp"
#include "./message.hpp"

#include <chrono>
#include <future>
#include <string>
#include <thread>

namespace http {

    /// Use this helper function to properly encode the "path" part of an URL.
    HTTP_API std::string encode_path(std::string const& path);

    /// Use this helper function to properly encode the "key" part of an URL parameter.
    HTTP_API std::string encode_key(std::string const& key);

    /// Use this helper function to properly encode the "value" part of an URL parameter.
    HTTP_API std::string encode_value(std::string const& value);

    /// Use this helper function to properly encode all chars in the given string.
    HTTP_API std::string encode_all(std::string const& value);

    /// Use this helper function to properly decode an encode string (path, key, or value).
    HTTP_API std::string decode(std::string const& str);

    /// Use this helper function to make all characters lower case.
    HTTP_API std::string to_lower(std::string str);

    /// Use this helper function to make all header keys lower case.
    HTTP_API http::headers to_lower(http::headers const& hdrs);

    /// This helper function is just provided as here due to consistency with
    /// respect to the wait_for() and wait_until() helper functions.
    template<typename FUTURE>
    inline void wait(FUTURE&& f) {
        f.wait();
    }

    /// Since the Microsoft STL implementation of std::future<T>::wait_for() is
    /// broken use this helper function instead which reimplements the functionality
    /// on Windows by a workaround and uses the correct implementation on the
    /// other platforms.
    template<typename FUTURE, typename TIME>
    inline auto wait_for(FUTURE&& f, TIME&& t) -> decltype(std::future_status::ready) {
#if !defined(WIN32)
        return f.wait_for(std::forward<TIME>(t));
#else // !defined(WIN32)
        return http::wait_until(std::forward<FUTURE>(f), std::chrono::system_clock::now() + duration);
#endif // !defined(WIN32)
    }

    /// Since the Microsoft STL implementation of std::future<T>::wait_until() is
    /// broken use this helper function instead which reimplements the functionality
    /// on Windows by a workaround and uses the correct implementation on the
    /// other platforms.
    template<typename FUTURE, typename TIME>
    inline auto wait_until(FUTURE&& f, TIME&& t) -> decltype(std::future_status::ready) {
#if !defined(WIN32)
        return f.wait_until(std::forward<TIME>(t));
#else // !defined(WIN32)
        while(true) {
            if(f._Is_ready()) { return std::future_status::ready; }
            if(std::chrono::system_clock::now() > t) { break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return std::future_status::timeout;
#endif // !defined(WIN32)
    }

} // namespace http
