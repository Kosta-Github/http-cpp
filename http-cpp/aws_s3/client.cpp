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

#include "./client.hpp"
#include "../oauth1/utils.hpp"

#include <ctime>
#include <cstring>

namespace {

    const char* const HTTP_PREFIX  = "http://s3.amazonaws.com/";
    const char* const HTTPS_PREFIX = "https://s3.amazonaws.com/";

    bool starts_with(const char* str, const char* pre) {
        assert(str);
        assert(pre);

        const auto lenStr = std::strlen(str);
        const auto lenPre = std::strlen(pre);
        return ((lenStr >= lenPre) && (std::strncmp(pre, str, lenPre) == 0));
    }

}

http::request http::aws_s3::client::request(
    http::url       url,
    http::operation op
) {
    // TODO implement virtual host names (https://mybucket.s3.amazonaws.com/)
    assert(starts_with(url.c_str(), HTTP_PREFIX) || starts_with(url.c_str(), HTTPS_PREFIX));
    const auto isHttps = starts_with(url.c_str(), HTTPS_PREFIX);

    // others need implementation :)
    assert(op == http::OP_GET());

    // bypass standard HTTP date header set by CuRL later on
    auto t  = std::time(nullptr);
    char dateBuf[64];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y%m%dT%H%M%SZ", std::gmtime(&t));
    headers["x-amz-date"] = dateBuf;

    std::string httpVerb = op;
    std::string contentMD5;
    std::string contentType;

    std::string canonicalizedAmzHeaders;
    for(auto&& header : headers) {
        const auto key = http::to_lower(header.first);
        if(starts_with(key.c_str(), "x-amz-")) {
            canonicalizedAmzHeaders += key + ":" + header.second + "\n";
        }
    }

    const auto canonicalizedResource = url.substr(std::strlen(isHttps ? HTTPS_PREFIX : HTTP_PREFIX) - 1);

    const auto stringToSign = 
        httpVerb                + "\n" +
        contentMD5              + "\n" +
        contentType             + "\n" +
        /*Date                  +*/ "\n" + // we are using "x-amz-date" instead
        canonicalizedAmzHeaders +
        canonicalizedResource;

    const auto signature = http::oauth1::create_signature(stringToSign, aws_secret_key);
    headers["authorization"] = "AWS " + aws_access_key_id + ":" + signature;

    return http::client::request(std::move(url), std::move(op));
}
