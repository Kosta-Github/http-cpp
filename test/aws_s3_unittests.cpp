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

#include <http-cpp/aws_s3/client.hpp>

CUTE_TEST( 
    "Test http::aws_s3::client",
    "[http],[aws_s3]"
) {
    http::aws_s3::client s3;
    if(const char* AWS_ACCESS_KEY_ID = getenv("AWS_ACCESS_KEY_ID"))
        s3.aws_access_key_id = AWS_ACCESS_KEY_ID;
    if(const char* AWS_SECRET_ACCESS_KEY = getenv("AWS_SECRET_ACCESS_KEY"))
        s3.aws_secret_key = AWS_SECRET_ACCESS_KEY;

    s3.connect_timeout = 10;
    s3.request_timeout = 60;
    
    auto message = s3.request(S3_TEST_URI).data().get();
    CUTE_ASSERT(message.status == http::HTTP_200_OK);
}