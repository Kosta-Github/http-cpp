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

#include "./client.hpp"

namespace http {
    namespace oauth1 {

        HTTP_API std::string create_timestamp();
        HTTP_API std::string create_nonce();

        HTTP_API http::parameters create_oauth_parameters(
            std::string consumer_key,
            std::string token_key,
            std::string timestamp,
            std::string nonce,
            std::string signature = ""
        );

        HTTP_API std::pair<std::string, std::string> create_oauth_header(
            http::parameters const& oauth_params
        );

        HTTP_API std::string create_signature_base(
            std::string const&      http_method,
            std::string const&      base_url,
            http::parameters const& params,
            std::string const&      consumer_key,
            std::string const&      token_key,
            std::string const&      timestamp,
            std::string const&      nonce
        );

        HTTP_API std::string create_signature(
            std::string const& sig_base,
            std::string const& consumer_secret,
            std::string const& token_secret
        );

        HTTP_API std::pair<std::string, std::string> create_oauth_header(
            client const&           client,
            http::url const&        url,
            http::operation const&  op,
            std::string const&      timestamp,
            std::string const&      nonce
        );

    } // namespace oauth1
} // namespace http
