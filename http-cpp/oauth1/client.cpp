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

static void add_oauth_params(
    http::parameters& inout_params,
    std::string consumer_key,
    std::string token_key,
    std::size_t seconds_in_epoche,
    std::string nonce
) {
    inout_params[OAUTH_VERSION] = "1.0";
    inout_params[OAUTH_SIGNATURE_METHOD] = "HMAC_SHA1";

    inout_params[OAUTH_CONSUMER_KEY] = std::move(consumer_key);

    if(!token_key.empty()) {
        inout_params[OAUTH_TOKEN] = std::move(token_key);
    }

    inout_params[OAUTH_TIMESTAMP] = std::to_string(seconds_in_epoche);
    inout_params[OAUTH_NONCE] = std::move(nonce);
}

static void add_url_params(
    http::parameters& inout_params,
    std::string const& url_params
) {
    if(url_params.empty()) { return; }

    auto last_amp = std::size_t(0);
    while(true) {
        auto next_amp = url_params.find('&', last_amp + 1);

        auto part = url_params.substr(last_amp, next_amp - last_amp);
        auto pos = part.find('=');
        if(pos > 0) {
            auto key =   http::decode((pos == part.npos) ? part : part.substr(0, pos));
            auto value = http::decode((pos == part.npos) ? ""   : part.substr(pos + 1));
            inout_params[std::move(key)] = std::move(value);
        }
    }
}

static void add_post_params(
    http::parameters& inout_params,
    http::form_data const& post_data
) {
    for(auto&& i : post_data) {
        inout_params[i.name] = i.content;
    }
}

static http::parameters encode_params(
    http::parameters const& params
) {
    http::parameters result;
    for(auto&& i : params) {
        result[http::encode_all(i.first)] = http::encode_all(i.second);
    }
    return result;
}

// needs to be improved!!!
static std::string create_nonce() {
    static std::atomic<std::size_t> s_counter(0);
    std::srand(std::time(nullptr) + ++s_counter);
    return std::to_string(std::rand());
}


static std::string create_signature_base(
    std::string const& http_method,
    std::string const& url,
    http::form_data const& post_data,
    std::string const& consumer_key,
    std::string const& token_key,
    std::size_t const  seconds_in_epoche,
    std::string const& nonce
) {
    // base on this description: https://dev.twitter.com/docs/auth/creating-signature

    // 1) collect HTTP method and convert it to all upper case chars
    auto method = http_method;
    std::transform(method.begin(), method.end(), method.begin(), std::toupper);

    // 2) get base URL (split at '?')
    auto pos = url.find('?');
    auto url_path   = http::decode((pos == url.npos) ? url : url.substr(0, pos));

    // 3) extract parameters from URL
    auto params = http::parameters();
    auto url_params = (pos == url.npos) ? "" : url.substr(pos + 1);
    add_url_params(params, url_params);

    // 4) extract parameters from POST data
    add_post_params(params, post_data);

    // 5) add oauth parameters
    add_oauth_params(params, consumer_key, token_key, seconds_in_epoche, nonce);

    // 6) percent-encode params and sort them thereafter(!) alphabethically
    auto encoded_params = http::parameters();
    for(auto&& i : params) {
        encoded_params[http::encode_all(i.first)] = http::encode_all(i.second);
    }

    // 7) create combined params string
    auto params_str = std::string();
    for(auto&& i : encoded_params) {
        if(!params_str.empty()) { params_str += '&'; }
        params_str += i.first;
        params_str += '=';
        params_str += i.second;
    }

    auto sig_base = std::string();
    sig_base += method;
    sig_base += '&';
    sig_base += http::encode_all(url_path);
    sig_base += '&';
    sig_base += http::encode_all(params_str);

    return sig_base;
}

static std::string calc_signature(
    std::string const& sig_base
) {
    return sig_base;
}

static std::string create_oauth_header() {
    return "";
}










http::request http::oauth1::client::request(
    http::url       url,
    http::operation op
) {
    auto time_stamp = std::time(nullptr);
    auto nonce = create_nonce();

    auto sig_base = create_signature_base(op, url, post_form, consumer.key, token.key, time_stamp, nonce);
    auto sig = calc_signature(sig_base);

    auto oauth_header = create_oauth_header();

    return http::client::request(std::move(url), std::move(op));
}
