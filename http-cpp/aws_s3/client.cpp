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
#include <iomanip>
#include <iostream>
#include <string.h>

namespace {
    bool starts_with(const char* str, const char* pre) 
    {
        size_t lenpre = strlen(pre);
        size_t lenstr = strlen(str);
        return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
    }

    const char* const HTTP_PREFIX = "http://s3.amazonaws.com/";
    const char* const HTTPS_PREFIX = "https://s3.amazonaws.com/";
}

http::request http::aws_s3::client::request(
    http::url       url,
    http::operation op
) {
    // TODO implement virtual host names (https://mybucket.s3.amazonaws.com/)
    assert(starts_with(url.c_str(), HTTP_PREFIX) || starts_with(url.c_str(), HTTPS_PREFIX));

    // others need implementation :)
    assert(op == http::OP_GET());

    // bypass standard HTTP date header set by CuRL later on
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::gmtime(&t);    
    char dateBuf[64];
    strftime(dateBuf, sizeof(dateBuf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    headers["X-Amz-Date"] = dateBuf;

    
    std::string httpVerb = "GET";
    std::string contentMD5 = "";
    std::string contentType = "";

    std::string canonicalizedAmzHeaders;
    for(auto&& header : headers)
    {
        if(starts_with(header.first.c_str(), "X-Amz-"))
        {
            std::string key = http::to_lower(header.first);
            canonicalizedAmzHeaders += key + ":" + header.second + "\n";
        }
    }

    std::string canonicalizedResource = url.substr(
        (starts_with(url.c_str(), HTTP_PREFIX) ? strlen(HTTP_PREFIX) : strlen(HTTPS_PREFIX)) - 1
    );

    std::string stringToSign = 
        httpVerb + "\n" +
        contentMD5 + "\n" +
        contentType + "\n" +
        /*Date +*/ "\n" + // we are using X-Amz-Date instead
        canonicalizedAmzHeaders +
        canonicalizedResource;

    std::string signature = http::oauth1::create_signature(stringToSign, aws_secret_key);
    headers["Authorization"] = "AWS " + aws_access_key_id + ":" + signature;

    return http::client::request(std::move(url), std::move(op));
}
