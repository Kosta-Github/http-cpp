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
    http::parameters& params,
    http::oauth1::consumer const& consumer,
    http::oauth1::token const& token,
    std::string signature,
    std::string nonce,
    std::string time_stamp
) {
    params[OAUTH_VERSION] = "1.0";

    params[OAUTH_CONSUMER_KEY] = consumer.key;

    if(token.key.empty()) {
        params.erase(OAUTH_TOKEN);
    } else {
        params[OAUTH_TOKEN] = token.key;
    }

    if(signature.empty()) {
        params.erase(OAUTH_SIGNATURE);
    } else {
        params[OAUTH_SIGNATURE] = std::move(signature);
    }
    params[OAUTH_SIGNATURE_METHOD] = "HMAC_SHA1";

    params[OAUTH_NONCE] = std::move(nonce);
    params[OAUTH_TIMESTAMP] = std::move(time_stamp);

}

static void split_params(
    http::parameters& inout_params,
    std::string const& str
) {
    if(str.empty()) { return; }

    auto last_amp = std::size_t(0);
    while(true) {
        auto next_amp = str.find('&', last_amp + 1);

        auto part = str.substr(last_amp, next_amp - last_amp);
        auto pos = part.find('=');
        if(pos > 0) {
            auto key =   (pos == part.npos) ? part : part.substr(0, pos);
            auto value = (pos == part.npos) ? ""   : part.substr(pos + 1);
            inout_params[std::move(key)] = std::move(value);
        }
    }
}

static std::string params_to_string(
    http::parameters const& params
) {
    std::string result;
    for(auto&& i : params) {
        if(!result.empty()) { result += '&'; }
        result += http::encode_key(i.first);
        result += '=';
        result += http::encode_value(i.second);
    }
    return result;
}

static std::string calc_oauth_signature(
    http::operation const& op,
    std::string const& url_path,
    http::parameters const& params
) {
    auto sig_base = std::string();
    sig_base += http::encode_all(op);
    sig_base += '&';
    sig_base += http::encode_all(http::decode(url_path));
    sig_base += '&';
    sig_base += http::encode_all(params_to_string(params));

    return sig_base;
}

http::request http::oauth1::client::request(
    http::url       url,
    http::operation op
) {
    // split URL into path and parameter parts at "?"
    auto pos = url.find('?');
    auto url_path   = (pos == url.npos) ? url : url.substr(0, pos);
    auto url_params = (pos == url.npos) ? ""  : url.substr(pos + 1);

    // extract the parameters
    auto params = http::parameters();
    split_params(params, url_params);

    // prepare the OAuth1 parameters, calculate the signature, and add it to the parameters
    auto nonce = "nonce";
    auto time_stamp = "time_stamp";

    add_oauth_headers(params, consumer, token, "", nonce, time_stamp);
    auto signature = calc_oauth_signature(op, url_path, params);
    add_oauth_headers(params, consumer, token, signature, nonce, time_stamp);

    // create the final query URL
    auto final_params = params_to_string(params);
    auto final_url = url_path + '?' + final_params;

    return http::client::request(std::move(final_url), std::move(op));
}
