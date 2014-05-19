//
// The MIT License (MIT)
//
// Copyright (c) 2013 by Konstantin (Kosta) Baumann & Autodesk Inc.
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

#include <cute/cute.hpp>

#include <http-cpp/oauth1/client.hpp>
#include <http-cpp/oauth1/utils.hpp>

CUTE_TEST(
    "Test http::oauth1::create_oauth_parameters() method work properly",
    "[http],[oauth1],[create_oauth_parameters]"
) {
    auto const consumer_key = "xvz1evFS4wEEPTGEFPHBog";
    auto const token_key    = "370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb";
    auto const timestamp    = "1318622958";
    auto const nonce        = "kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg";
    auto const signature    = "tnnArxj06cWHq44gCs1OSKk/jLY=";
    auto const sig_method   = "HMAC-SHA1";
    auto const version      = "1.0";

    auto const header_key_expected      = "Authorization";
    auto const header_value_expected    = ""
        "OAuth "
        "oauth_consumer_key=\"xvz1evFS4wEEPTGEFPHBog\", "
        "oauth_nonce=\"kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg\", "
        "oauth_signature=\"tnnArxj06cWHq44gCs1OSKk%2FjLY%3D\", "
        "oauth_signature_method=\"HMAC-SHA1\", "
        "oauth_timestamp=\"1318622958\", "
        "oauth_token=\"370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb\", "
        "oauth_version=\"1.0\"";

    auto params = http::oauth1::create_oauth_parameters(
        consumer_key, token_key, timestamp, nonce, signature
    );

    auto header = http::oauth1::create_oauth_header(params);

    CUTE_ASSERT(params["oauth_consumer_key"]        == consumer_key);
    CUTE_ASSERT(params["oauth_token"]               == token_key);
    CUTE_ASSERT(params["oauth_timestamp"]           == timestamp);
    CUTE_ASSERT(params["oauth_nonce"]               == nonce);
    CUTE_ASSERT(params["oauth_signature"]           == signature);
    CUTE_ASSERT(params["oauth_signature_method"]    == sig_method);
    CUTE_ASSERT(params["oauth_version"]             == version);

    CUTE_ASSERT(header.first    == header_key_expected);
    CUTE_ASSERT(header.second   == header_value_expected);
}

CUTE_TEST(
    "Test http::oauth1::create_signature_base() method work properly",
    "[http],[oauth1],[create_signature_base]"
) {
    auto const http_method  = http::OP_POST();
    auto const base_url     = "https://api.twitter.com/1/statuses/update.json";
    auto const consumer_key = "xvz1evFS4wEEPTGEFPHBog";
    auto const token_key    = "370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb";
    auto const timestamp    = "1318622958";
    auto const nonce        = "kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg";

    auto params = http::parameters();
    params["include_entities"]  = "true";
    params["status"]            = "Hello Ladies + Gentlemen, a signed OAuth request!";

    auto const sig_base_expected = ""
        "POST&"
        "https%3A%2F%2Fapi.twitter.com%2F1%2Fstatuses%2Fupdate.json&"
        "include_entities%3Dtrue%26"
        "oauth_consumer_key%3Dxvz1evFS4wEEPTGEFPHBog%26"
        "oauth_nonce%3DkYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg%26"
        "oauth_signature_method%3DHMAC-SHA1%26"
        "oauth_timestamp%3D1318622958%26"
        "oauth_token%3D370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb%26"
        "oauth_version%3D1.0%26"
        "status%3DHello%2520Ladies%2520%252B%2520Gentlemen%252C%2520a%2520signed%2520OAuth%2520request%2521";

    auto sig_base = http::oauth1::create_signature_base(
        http_method, base_url, params, consumer_key, token_key, timestamp, nonce
    );

    CUTE_ASSERT(sig_base == sig_base_expected);
}

CUTE_TEST(
    "Test http::oauth1::create_signature() method work properly",
    "[http],[oauth1],[create_signature]"
) {
    auto const consumer_secret  = "kAcSOqF21Fu85e7zjz7ZN2U4ZRhfV3WpwPAoE3Z7kBw";
    auto const token_secret     = "LswwdoUaIvS8ltyTt5jkRh4J50vUPVVHtR2YPi5kE";
    auto const sig_expected     = "tnnArxj06cWHq44gCs1OSKk/jLY=";
    auto const sig_base         = ""
        "POST&"
        "https%3A%2F%2Fapi.twitter.com%2F1%2Fstatuses%2Fupdate.json&"
        "include_entities%3Dtrue%26"
        "oauth_consumer_key%3Dxvz1evFS4wEEPTGEFPHBog%26"
        "oauth_nonce%3DkYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg%26"
        "oauth_signature_method%3DHMAC-SHA1%26"
        "oauth_timestamp%3D1318622958%26"
        "oauth_token%3D370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb%26"
        "oauth_version%3D1.0%26"
        "status%3DHello%2520Ladies%2520%252B%2520Gentlemen%252C%2520a%2520signed%2520OAuth%2520request%2521";

    auto sig = http::oauth1::create_signature(
        sig_base, consumer_secret, token_secret
    );
    CUTE_ASSERT(sig == sig_expected);
}

CUTE_TEST(
    "Test http::oauth1::create_oauth_header() method works properly for a fully configured client object and a POST request",
    "[http],[oauth1],[create_oauth_header][POST]"
) {
    auto client = http::oauth1::client();
    client.consumer_key     = "xvz1evFS4wEEPTGEFPHBog";
    client.consumer_secret  = "kAcSOqF21Fu85e7zjz7ZN2U4ZRhfV3WpwPAoE3Z7kBw";
    client.token_key        = "370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb";
    client.token_secret     = "LswwdoUaIvS8ltyTt5jkRh4J50vUPVVHtR2YPi5kE";
    auto const timestamp    = "1318622958";
    auto const nonce        = "kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg";
    auto const http_method  = http::OP_POST();
    auto const url          = ""
        "https://api.twitter.com/1/statuses/update.json?"
        "include_entities=true&"
        "status=Hello%20Ladies%20%2B%20Gentlemen%2C%20a%20signed%20OAuth%20request%21";

    auto const header_key_expected      = "Authorization";
    auto const header_value_expected    = ""
        "OAuth "
        "oauth_consumer_key=\"xvz1evFS4wEEPTGEFPHBog\", "
        "oauth_nonce=\"kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg\", "
        "oauth_signature=\"tnnArxj06cWHq44gCs1OSKk%2FjLY%3D\", "
        "oauth_signature_method=\"HMAC-SHA1\", "
        "oauth_timestamp=\"1318622958\", "
        "oauth_token=\"370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb\", "
        "oauth_version=\"1.0\"";

    auto const header = http::oauth1::create_oauth_header(
        client, url, http_method, timestamp, nonce
    );

    CUTE_ASSERT(header.first    == header_key_expected);
    CUTE_ASSERT(header.second   == header_value_expected);
}
