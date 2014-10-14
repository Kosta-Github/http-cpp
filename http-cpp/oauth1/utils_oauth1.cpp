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

#include "./utils.hpp"
#include "../utils.hpp"
#include "../../3rdParty/base64/base64.h"
#include "../../3rdParty/HMAC_SHA1/HMAC_SHA1.h"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <random>

// based on this description: https://dev.twitter.com/docs/auth/creating-signature

static const char* const OAUTH_VERSION          = "oauth_version";
static const char* const OAUTH_CONSUMER_KEY     = "oauth_consumer_key";
static const char* const OAUTH_TOKEN            = "oauth_token";
static const char* const OAUTH_SIGNATURE_METHOD = "oauth_signature_method";
static const char* const OAUTH_SIGNATURE        = "oauth_signature";
static const char* const OAUTH_NONCE            = "oauth_nonce";
static const char* const OAUTH_TIMESTAMP        = "oauth_timestamp";

static const char* const AUTH_HEADER_FIELD      = "Authorization";
static const char* const AUTH_HEADER_PREFIX     = "OAuth ";

std::string http::oauth1::create_timestamp() {
    return std::to_string(std::time(nullptr));
}

// needs to be improved!!!
std::string http::oauth1::create_nonce() {
    std::string random_values(32, '\0');

    {   // need to protect the static state with a mutex
        static std::mutex s_mutex;
        std::lock_guard<std::mutex> lock(s_mutex);

        static std::mt19937 eng;
        static std::uniform_int_distribution<char> dist;
        static auto random_gen = [&]() { return dist(eng); };

        std::generate(random_values.begin(), random_values.end(), random_gen);
    }

    auto random_base64 = base64_encode(
        reinterpret_cast<const unsigned char*>(random_values.c_str()),
        static_cast<unsigned int>(random_values.size())
    );

    return http::encode_all(random_base64);
}

http::parameters http::oauth1::create_oauth_parameters(
    std::string consumer_key,
    std::string token_key,
    std::string timestamp,
    std::string nonce,
    std::string signature
) {
    auto params = http::parameters();

    params[OAUTH_VERSION] = "1.0";
    params[OAUTH_SIGNATURE_METHOD] = "HMAC-SHA1";

    params[OAUTH_CONSUMER_KEY] = std::move(consumer_key);

    if(!token_key.empty()) {
        params[OAUTH_TOKEN] = std::move(token_key);
    }

    params[OAUTH_TIMESTAMP] = std::move(timestamp);
    params[OAUTH_NONCE] = std::move(nonce);

    if(!signature.empty()) {
        params[OAUTH_SIGNATURE] = std::move(signature);
    }

    return params;
}

std::pair<std::string, std::string> http::oauth1::create_oauth_header(
    http::parameters const& params
) {
    auto str = std::string();
    for(auto&& i : params) {
        if(!str.empty()) { str += ", "; }
        str += i.first;
        str += "=\"" + encode_all(i.second) + "\"";
    }

    return std::make_pair(AUTH_HEADER_FIELD, AUTH_HEADER_PREFIX + str);
}

std::string http::oauth1::create_signature_base(
    std::string const& http_method,
    std::string const& url_base,
    http::parameters const& request_params,
    std::string const& consumer_key,
    std::string const& token_key,
    std::string const& timestamp,
    std::string const& nonce
) {
    // collect all parameters (URL params & oauth params), percent-encode them (keys and values),
    // and sort the precent-encoded keys
    auto params = http::parameters();
    for(auto&& i : request_params) {
        params[http::encode_all(i.first)] = http::encode_all(i.second);
    }

    // add the oauth params (without the signature)
    for(auto&& i : http::oauth1::create_oauth_parameters(consumer_key, token_key, timestamp, nonce)) {
        params[http::encode_all(i.first)] = http::encode_all(i.second);
    }

    // create combined params string
    auto params_str = std::string();
    for(auto&& i : params) {
        if(!params_str.empty()) { params_str += '&'; }
        params_str += i.first;
        params_str += '=';
        params_str += i.second;
    }

    auto sig_base = std::string();
    sig_base += http::encode_all(http_method);
    sig_base += '&';
    sig_base += http::encode_all(url_base);
    sig_base += '&';
    sig_base += http::encode_all(params_str);

    return sig_base;
}

std::string http::oauth1::create_signature(
    std::string const& sig_base,
    std::string const& consumer_secret,
    std::string const& token_secret
) {
    // concatinate consumer secret and token secret with "&"
    auto sign_key = std::string();
    sign_key += encode_all(consumer_secret);
    sign_key += '&';
    sign_key += encode_all(token_secret);

    return create_signature(sig_base, sign_key);
}

std::string http::oauth1::create_signature(
    std::string const& sig_base,
    std::string const& sign_key
) {
    // create HMAC-SHA1 digest
    auto digest = std::string(CHMAC_SHA1::SHA1_DIGEST_LENGTH, 0x00);
    CHMAC_SHA1 sha1;
    sha1.HMAC_SHA1(
        (BYTE*)&sig_base[0], static_cast<int>(sig_base.size()),
        (BYTE*)&sign_key[0], static_cast<int>(sign_key.size()),
        (BYTE*)&digest[0]
    );

    // return base64 encode SHA1 digest
    return base64_encode(
        reinterpret_cast<const unsigned char*>(digest.c_str()),
        CHMAC_SHA1::SHA1_DIGEST_LENGTH
    );
}

static http::parameters extract_url_params(
    std::string const& url_params
) {
    auto params = http::parameters();
    if(url_params.empty()) { return params; }

    auto last_amp = std::size_t(0);
    while(true) {
        auto next_amp = url_params.find('&', last_amp);

        auto part = url_params.substr(last_amp, next_amp - last_amp);
        auto pos = part.find('=');
        if(pos > 0) {
            auto key =   http::decode((pos == part.npos) ? part : part.substr(0, pos));
            auto value = http::decode((pos == part.npos) ? ""   : part.substr(pos + 1));
            params[std::move(key)] = std::move(value);
        }

        if(next_amp == url_params.npos) {
            break;
        }

        last_amp = next_amp + 1;
    }

    return params;
}

static http::parameters extract_post_params(
    http::form_data const& post_form
) {
    auto params = http::parameters();
    for(auto&& i : post_form) {
        params[i.name] = i.content;
    }
    return params;
}

std::pair<std::string, std::string> http::oauth1::create_oauth_header(
    client const&           client,
    http::url const&        url,
    http::operation const&  op,
    std::string const&      timestamp,
    std::string const&      nonce
) {
    // collect all paramaters
    auto params = http::parameters();

    // 1) get base URL (split at '?')
    auto pos = url.find('?');
    auto url_base = http::decode((pos == url.npos) ? url : url.substr(0, pos));

    // 2) extract parameters from URL
    auto url_params = (pos == url.npos) ? "" : url.substr(pos + 1);
    for(auto&& i : extract_url_params(url_params)) {
        params[i.first] = i.second;
    }

    // 3) extract parameters from POST data
    for(auto&& i : extract_post_params(client.post_form)) {
        params[i.first] = i.second;
    }

    auto sig_base = http::oauth1::create_signature_base(
        op, url_base, params,
        client.consumer_key, client.token_key,
        timestamp, nonce
    );
    auto sig = http::oauth1::create_signature(
        sig_base, client.consumer_secret, client.token_secret
    );
    auto oauth_params = http::oauth1::create_oauth_parameters(
        client.consumer_key, client.token_key, timestamp, nonce, sig
    );

    return http::oauth1::create_oauth_header(oauth_params);
}
