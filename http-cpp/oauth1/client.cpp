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

#include "client.hpp"

static const char* const OAUTH_VERSION          = "oauth_version";
static const char* const OAUTH_CONSUMER_KEY     = "oauth_consumer_key";
static const char* const OAUTH_TOKEN            = "oauth_token";
static const char* const OAUTH_TOKEN_SECRET     = "oauth_token_secret";
static const char* const OAUTH_SIGNATURE_METHOD = "oauth_signature_method";
static const char* const OAUTH_SIGNATURE        = "oauth_signature";
static const char* const OAUTH_NONCE            = "oauth_nonce";
static const char* const OAUTH_TIMESTAMP        = "oauth_timestamp";
static const char* const OAUTH_VERIFIER         = "oauth_verifier";
static const char* const OAUTH_CALLBACK         = "oauth_callback";

static const char* const AUTHHEADER_FIELD       = "Authorization: ";
static const char* const AUTHHEADER_PREFIX      = "OAuth ";

http::oauth1::key_secret::key_secret(std::string key_, std::string secret_) : key(std::move(key_)), secret(std::move(secret_)) { }

http::oauth1::client::client(oauth1::consumer consumer_, oauth1::token token_) : consumer(std::move(consumer_)), token(std::move(token_)) { }

static void add_oauth_headers(
    http::headers& headers,
    http::oauth1::consumer const& consumer,
    http::oauth1::token const& token,
    std::string signature,
    std::string nonce,
    std::string time_stamp
) {
    headers[OAUTH_VERSION] = "1.0";

    headers[OAUTH_CONSUMER_KEY] = consumer.key;

    if(!token.key.empty()) {
        headers[OAUTH_TOKEN] = token.key;
    }

    if(!signature.empty()) {
        headers[OAUTH_SIGNATURE] = std::move(signature);
    }
    headers[OAUTH_SIGNATURE_METHOD] = "HMAC_SHA1";

    headers[OAUTH_NONCE] = std::move(nonce);
    headers[OAUTH_TIMESTAMP] = std::move(time_stamp);

}



http::request http::oauth1::client::request(
    http::url       url,
    http::operation op
) {
    add_oauth_headers(headers, consumer, token, "signature", "nonce", "time_stamp");

    return http::client::request(std::move(url), std::move(op));
}
