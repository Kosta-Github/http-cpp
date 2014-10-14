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

#include "../client.hpp"

 // disable warning: class 'ABC' needs to have dll-interface to be used by clients of struct 'XYZ'
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif // defined(_MSC_VER)

namespace http {
    namespace aws_s3 {

        struct HTTP_API client : public http::client {
            std::string aws_access_key_id;
            std::string aws_secret_key;

            virtual http::request request(
                http::url       url,
                http::operation op = http::OP_GET()
            ) override;
        };

    } // namespace oauth1
} // namespace http

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif // defined(_MSC_VER)
