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

#include <http-cpp/utils.hpp>

#include <map>

CUTE_TEST(
    "Test http::encode_path() method work properly",
    "[http],[encode_path]"
) {
    std::map<std::string, std::string> tests;
    tests[" "] = "%20";
    tests["!"] = "%21";
    tests["#"] = "%23";
    tests["$"] = "%24";
    tests["&"] = "&";
    tests["'"] = "%27";
    tests["("] = "%28";
    tests[")"] = "%29";
    tests["*"] = "%2A";
    tests["+"] = "+";
    tests[","] = "%2C";
    tests["/"] = "/";
    tests[":"] = "%3A";
    tests[";"] = "%3B";
    tests["="] = "=";
    tests["?"] = "%3F";
    tests["@"] = "%40";
    tests["["] = "%5B";
    tests["]"] = "%5D";
    tests["hello world"] = "hello%20world";
    tests["hello world/hello universe"] = "hello%20world/hello%20universe";
    tests["http://hello world/hello universe"] = "http://hello%20world/hello%20universe";
    tests["https://hello world/hello universe"] = "https://hello%20world/hello%20universe";

    for(auto&& t : tests) {
        auto&& orig    = t.first;
        auto&& encoded = t.second;
        CUTE_ASSERT(http::encode_path(orig) == encoded, CUTE_CAPTURE(orig));
        CUTE_ASSERT(http::decode(encoded) == orig, CUTE_CAPTURE(encoded));
    }
}

CUTE_TEST(
    "Test http::encode_key() method work properly",
    "[http],[encode_key]"
) {
    std::map<std::string, std::string> tests;
    tests[" "] = "%20";
    tests["!"] = "%21";
    tests["#"] = "%23";
    tests["$"] = "%24";
    tests["&"] = "%26";
    tests["'"] = "%27";
    tests["("] = "%28";
    tests[")"] = "%29";
    tests["*"] = "%2A";
    tests["+"] = "%2B";
    tests[","] = "%2C";
    tests["/"] = "%2F";
    tests[":"] = "%3A";
    tests[";"] = "%3B";
    tests["="] = "%3D";
    tests["?"] = "%3F";
    tests["@"] = "%40";
    tests["["] = "%5B";
    tests["]"] = "%5D";

    for(auto&& t : tests) {
        auto&& orig    = t.first;
        auto&& encoded = t.second;
        CUTE_ASSERT(http::encode_key(orig) == encoded, CUTE_CAPTURE(orig));
        CUTE_ASSERT(http::decode(encoded) == orig, CUTE_CAPTURE(encoded));
    }
}

CUTE_TEST(
    "Test http::encode_value() method work properly",
    "[http],[encode_value]"
) {
    std::map<std::string, std::string> tests;
    tests[" "] = "%20";
    tests["!"] = "%21";
    tests["#"] = "%23";
    tests["$"] = "%24";
    tests["&"] = "%26";
    tests["'"] = "%27";
    tests["("] = "%28";
    tests[")"] = "%29";
    tests["*"] = "%2A";
    tests["+"] = "%2B";
    tests[","] = "%2C";
    tests["/"] = "%2F";
    tests[":"] = "%3A";
    tests[";"] = "%3B";
    tests["="] = "=";
    tests["?"] = "%3F";
    tests["@"] = "%40";
    tests["["] = "%5B";
    tests["]"] = "%5D";

    for(auto&& t : tests) {
        auto&& orig    = t.first;
        auto&& encoded = t.second;
        CUTE_ASSERT(http::encode_value(orig) == encoded, CUTE_CAPTURE(orig));
        CUTE_ASSERT(http::decode(encoded) == orig, CUTE_CAPTURE(encoded));
    }
}

CUTE_TEST(
    "Test http::encode_all() method work properly",
    "[http],[encode_all]"
) {
    std::map<std::string, std::string> tests;
    tests[" "] = "%20";
    tests["!"] = "%21";
    tests["#"] = "%23";
    tests["$"] = "%24";
    tests["&"] = "%26";
    tests["'"] = "%27";
    tests["("] = "%28";
    tests[")"] = "%29";
    tests["*"] = "%2A";
    tests["+"] = "%2B";
    tests[","] = "%2C";
    tests["/"] = "%2F";
    tests[":"] = "%3A";
    tests[";"] = "%3B";
    tests["="] = "%3D";
    tests["?"] = "%3F";
    tests["@"] = "%40";
    tests["["] = "%5B";
    tests["]"] = "%5D";

    tests["Ladies + Gentlemen"] = "Ladies%20%2B%20Gentlemen";
    tests["An encoded string!"] = "An%20encoded%20string%21";
    tests["Dogs, Cats & Mice"] = "Dogs%2C%20Cats%20%26%20Mice";

    for(auto&& t : tests) {
        auto&& orig    = t.first;
        auto&& encoded = t.second;
        CUTE_ASSERT(http::encode_all(orig) == encoded, CUTE_CAPTURE(orig));
        CUTE_ASSERT(http::decode(encoded) == orig, CUTE_CAPTURE(encoded));
    }
}
