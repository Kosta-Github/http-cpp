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

#include <catch/catch.hpp>

#include <http-cpp/client.hpp>
#include <http-cpp/requests.hpp>
#include <http-cpp/utils.hpp>

#include <atomic>

static const std::string LOCALHOST = "http://localhost:8888/";

CATCH_TEST_CASE(
    "Tests HTTP_200_OK status for a localhost web-server",
    "[http][request][200][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    auto data = http::client().request(url).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body == "URL found");
}

CATCH_TEST_CASE(
    "Tests HTTP_404_NOT_FOUND status for a localhost web-server",
    "[http][request][404][localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    auto data = http::client().request(url).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_404_NOT_FOUND);
    CATCH_CHECK(data.body == "URL not found");
}

CATCH_TEST_CASE(
    "Tests canceling a running requestfor a localhost web-server",
    "[http][request][cancel][localhost]"
) {
    auto start_time = std::chrono::system_clock::now();

    auto url = LOCALHOST + "delay";
    auto request = http::client().request(url);
    request.cancel(); // cancel the request immediately
    auto data = request.data().get();

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_CANCELED);
    CATCH_CHECK(duration <= 500); // should be clearly below the 1 second the web server will delay to answer the request
}

CATCH_TEST_CASE(
    "Tests a request on a non-open port on localhost",
    "[http][request][non-open-port][localhost]"
) {
    auto url = "http://localhost:1/";
    auto data = http::client().request(url).data().get();

    CATCH_CHECK(data.error_code > 0);
}

CATCH_TEST_CASE(
    "Tests a request to a non-existing hostname",
    "[http][request][invalid-hostname]"
) {
    auto url = "http://abc.xyz/";
    auto data = http::client().request(url).data().get();

    CATCH_CHECK(data.error_code > 0);
}

CATCH_TEST_CASE(
    "Tests a GET request for a localhost web-server",
    "[http][request][GET][localhost]"
) {
    auto url = LOCALHOST + "get_request";
    auto data = http::client().request(url, http::HTTP_GET).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body == "GET received");
}

CATCH_TEST_CASE(
    "Tests a HEAD request for a localhost web-server",
    "[http][request][HEAD][localhost]"
) {
    auto url = LOCALHOST + "head_request";
    auto data = http::client().request(url, http::HTTP_HEAD).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body.empty()); // the body needs to be empty for a HEAD request
}

/*
CATCH_TEST_CASE(
    "Tests a POST request for a localhost web-server",
    "[http][request][POST][localhost]"
) {
    auto url = LOCALHOST + "post_request";
    auto data = http::client().request(url, http::HTTP_POST).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body == "POST received");
}
*/

CATCH_TEST_CASE(
    "Tests a PUT request for a localhost web-server",
    "[http][request][PUT][localhost]"
) {
    auto url = LOCALHOST + "put_request";
    auto put_data = std::string("I am the PUT workload!");
    auto data = http::client().request(url, http::HTTP_PUT, http::headers(), put_data).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body == ("PUT received: " + put_data));
}

CATCH_TEST_CASE(
    "Tests a DELETE request for a localhost web-server",
    "[http][request][DELETE][localhost]"
) {
    auto url = LOCALHOST + "delete_request";
    auto data = http::client().request(url, http::HTTP_DELETE).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);
    CATCH_CHECK(data.body == "DELETE received");
}

CATCH_TEST_CASE(
    "Tests to round trip HTTP header data for a localhost web-server",
    "[http][request][headers][localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    auto headers_send = http::headers();
    headers_send["http-cpp"] = "is cool";
    headers_send["cool"]     = "is http-cpp";
    auto data = http::client().request(url, http::HTTP_GET, headers_send).data().get();

    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);
    CATCH_CHECK(data.status == http::HTTP_200_OK);

    auto headers_received = data.headers;
    for(auto&& h : headers_send) {
        CATCH_CHECK(h.second == headers_received[h.first]);
    }
}

static void perform_parallel_requests(
    const size_t count,
    const http::url url,
    const http::error_code expected_error_code,
    const http::status expected_status,
    const std::string& expected_message
) {
    auto client = http::client();
    auto reqs = http::requests();

    for(size_t i = 0; i < count; ++i) {
        reqs.request(client, url);
    }

    for(size_t i = 0; i < count; ++i) {
        auto data = reqs.reqs[i].data().get();
        CATCH_CHECK(data.error_code == expected_error_code);
        CATCH_CHECK(data.status == expected_status);
        CATCH_CHECK(data.body == expected_message);
    }

    reqs.cancel_all();
}

CATCH_TEST_CASE(
    "Test multiple parallel requests against localhost web-server",
    "[http][requests][200][localhost]"
) {
    perform_parallel_requests(
        10,
        LOCALHOST + "HTTP_200_OK",
        http::HTTP_REQUEST_FINISHED,
        http::HTTP_200_OK,
        "URL found"
    );
}

CATCH_TEST_CASE(
    "Test multiple parallel requests against localhost web-server for a non-existing URL",
    "[http][requests][404][localhost]"
) {
    perform_parallel_requests(
        10,
        LOCALHOST + "HTTP_404_NOT_FOUND",
        http::HTTP_REQUEST_FINISHED,
        http::HTTP_404_NOT_FOUND,
        "URL not found"
    );
}

static void perform_parallel_stream_requests(
    const int count,
    const http::url& url,
    const http::status expected_status
) {
    auto client = http::client();

    std::atomic<int> active_count(count);

    for(int i = 0; i < count; ++i) {
        auto cb = [&](http::message msg, http::progress progress) -> bool {
            switch(msg.error_code) {
                case http::HTTP_REQUEST_PROGRESS: {
                    break;
                }
                case http::HTTP_REQUEST_FINISHED: {
                    CATCH_CHECK(msg.status == expected_status);
                    --active_count;
                    break;
                }
                default: {
                    CATCH_REQUIRE(msg.error_code == http::HTTP_REQUEST_PROGRESS);
                    break;
                }
            }
            return true;
        };
        
        client.request_stream(cb, url, http::HTTP_GET);
    }

    while(active_count > 0) {
        std::this_thread::yield();
    }

    client.cancel_all();
}

CATCH_TEST_CASE(
    "Test multiple parallel streaming requests against localhost web-server",
    "[http][requests][stream][localhost]"
) {
    perform_parallel_stream_requests(10, LOCALHOST + "stream", http::HTTP_200_OK);
}

CATCH_TEST_CASE(
    "Test multiple parallel streaming requests against google server",
    "[http][requests][stream][google]"
) {
    perform_parallel_stream_requests(10, "https://www.google.com/?gws_rd=cr#q=Autodesk", http::HTTP_200_OK);
}



CATCH_TEST_CASE(
    "test http::escape()",
    "[http][escape]"
) {
    CATCH_CHECK(http::escape("hello world") == "hello%20world");
    CATCH_CHECK(http::escape("<>&?%/\\:=*") == "%3C%3E%26%3F%25%2F%5C%3A%3D%2A");
}

CATCH_TEST_CASE(
    "test http::unescape()",
    "[http][unescape]"
) {
    CATCH_CHECK(http::unescape("hello%20world") == "hello world");
    CATCH_CHECK(http::unescape("%3C%3E%26%3F%25%2F%5C%3A%3D%2A") == "<>&?%/\\:=*");
}
