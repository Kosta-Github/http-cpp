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

static bool contains(std::string const& str, std::string const& find) {
    return (str.find(find) != str.npos);
}

static inline void check_result(
    http::message const& data,
    std::string const& expected_body,
    http::error_code const expected_error_code = http::HTTP_REQUEST_FINISHED,
    http::status const expected_status = http::HTTP_200_OK
) {
    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code == expected_error_code);

    CATCH_CAPTURE(http::status_to_string(data.status));
    CATCH_CHECK(data.status == expected_status);

    CATCH_CHECK(data.body == expected_body);
}

static const std::string LOCALHOST = "http://localhost:8888/";

CATCH_TEST_CASE(
    "Tests HTTP_200_OK status for a localhost web-server",
    "[http][request][200][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    check_result(http::client().request(url).data().get(), "URL found");
}

CATCH_TEST_CASE(
    "Tests HTTP_404_NOT_FOUND status for a localhost web-server",
    "[http][request][404][localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    check_result(
        http::client().request(url).data().get(),
        "URL not found",
        http::HTTP_REQUEST_FINISHED,
        http::HTTP_404_NOT_FOUND
    );
}

CATCH_TEST_CASE(
    "Tests canceling a running request for a localhost web-server",
    "[http][request][cancel][localhost]"
) {
    auto start_time = std::chrono::system_clock::now();

    auto url = LOCALHOST + "delay";
    auto request = http::client().request(url);
    request.cancel(); // cancel the request immediately
    auto data = request.data().get();

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_CANCELED);
    CATCH_CHECK(duration < 3); // should be clearly below the 3 seconds the web server will delay to answer the request
}

CATCH_TEST_CASE(
    "Tests request timeout for a localhost web-server",
    "[http][request][timeout][localhost]"
) {
    auto start_time = std::chrono::system_clock::now();

    auto url = LOCALHOST + "delay";
    auto client = http::client();
    client.request_timeout = 1; // 1 second timeout
    auto data = client.request(url).data().get();

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code > 0);
    CATCH_CHECK(duration < 3); // should be clearly below the 3 second the web server will delay to answer the request
}

CATCH_TEST_CASE(
    "Tests a request on a non-open port on localhost",
    "[http][request][non-open-port][localhost]"
) {
    auto url = "http://localhost:1/";
    auto data = http::client().request(url).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code > 0);
}

CATCH_TEST_CASE(
    "Tests a request to a non-existing hostname",
    "[http][request][invalid-hostname]"
) {
    auto url = "http://abc.xyz/";
    auto data = http::client().request(url).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code > 0);
}

CATCH_TEST_CASE(
    "Tests a GET request for a localhost web-server",
    "[http][request][GET][localhost]"
) {
    auto url = LOCALHOST + "get_request";
    check_result(http::client().request(url, http::HTTP_GET).data().get(), "GET received");
}

CATCH_TEST_CASE(
    "Tests a HEAD request for a localhost web-server",
    "[http][request][HEAD][localhost]"
) {
    auto url = LOCALHOST + "head_request";
    check_result(http::client().request(url, http::HTTP_HEAD).data().get(), ""); // the body needs to be empty for a HEAD request
}

CATCH_TEST_CASE(
    "Tests a POST request for a localhost web-server",
    "[http][request][POST][localhost]"
) {
    auto url = LOCALHOST + "post_request";

    auto post_data = http::post_data();
    post_data["library"]    = "http-cpp";
    post_data["color"]      = "red";
    post_data["age"]        = "42";

    auto data = http::client().request(url, http::HTTP_POST, http::headers(), http::buffer(), post_data).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);

    CATCH_CAPTURE(http::status_to_string(data.status));
    CATCH_CHECK(data.status == http::HTTP_200_OK);

    CATCH_CAPTURE(data.body);
    CATCH_CHECK(contains(data.body, "POST received: "));

    for(auto&& i : post_data) {
        auto find = "Content-Disposition: form-data; name=\"" + i.first + "\"";
        CATCH_CAPTURE(find);
        CATCH_CHECK(contains(data.body, find));
    }
}

CATCH_TEST_CASE(
    "Tests a PUT request for a localhost web-server",
    "[http][request][PUT][localhost]"
) {
    auto url = LOCALHOST + "put_request";
    auto put_data = std::string("I am the PUT workload!");
    check_result(http::client().request(url, http::HTTP_PUT, http::headers(), put_data).data().get(), "PUT received: " + put_data);
}

CATCH_TEST_CASE(
    "Tests a DELETE request for a localhost web-server",
    "[http][request][DELETE][localhost]"
) {
    auto url = LOCALHOST + "delete_request";
    check_result(http::client().request(url, http::HTTP_DELETE).data().get(), "DELETE received");
}

CATCH_TEST_CASE(
    "Tests round tripping HTTP header data for a localhost web-server",
    "[http][request][headers][localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    auto headers_req = http::headers();
    headers_req["http-cpp"] = "is cool";
    headers_req["cool"]     = "is http-cpp";
    auto data = http::client().request(url, http::HTTP_GET, headers_req).data().get();

    check_result(data, "headers received");

    auto headers_received = data.headers;
    for(auto&& h : headers_req) {
        CATCH_CHECK(h.second == headers_received[h.first]);
    }
}

CATCH_TEST_CASE(
    "Tests merging headers set in the client object for a localhost web-server",
    "[http][request][headers][merge][localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    // add an header entry to the client object
    auto client = http::client();
    client.headers["http-cpp"] = "is cool";

    // create a second headers object specific for the request
    // and add another header entry to that one
    auto headers_req = http::headers();
    headers_req["cool"] = "is http-cpp";

    // perform the request
    auto data = client.request(url, http::HTTP_GET, headers_req).data().get();

    check_result(data, "headers received");

    // check that the server received (and echoed) both header entries
    auto headers_received = data.headers;
    for(auto&& h : client.headers) {
        CATCH_CHECK(h.second == headers_received[h.first]);
    }
    for(auto&& h : headers_req) {
        CATCH_CHECK(h.second == headers_received[h.first]);
    }
}

CATCH_TEST_CASE(
    "Tests overwriting headers set in the client object works",
    "[http][request][headers][merge][localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    // add an header entry to the client object
    auto client = http::client();
    client.headers["color"] = "red";

    // create a second headers object specific for the request
    // and add another header entry to that one
    auto headers_req = http::headers();
    headers_req["color"] = "green";

    // perform the request
    auto data1 = client.request(url, http::HTTP_GET             ).data().get();
    auto data2 = client.request(url, http::HTTP_GET, headers_req).data().get();
    auto data3 = client.request(url, http::HTTP_GET             ).data().get();
    check_result(data1, "headers received");
    check_result(data2, "headers received");
    check_result(data3, "headers received");

    // check that the server received the correct values
    auto headers_received1 = data1.headers;
    auto headers_received2 = data2.headers;
    auto headers_received3 = data3.headers;
    CATCH_CHECK(headers_received1["color"] == "red");
    CATCH_CHECK(headers_received2["color"] == "green");
    CATCH_CHECK(headers_received3["color"] == "red");
}

static void perform_parallel_requests(
    const size_t count,
    const http::url url,
    const std::string& expected_message,
    const http::error_code expected_error_code = http::HTTP_REQUEST_FINISHED,
    const http::status expected_status = http::HTTP_200_OK
) {
    auto client = http::client();
    auto reqs = http::requests();

    for(size_t i = 0; i < count; ++i) {
        reqs.request(client, url);
    }

    for(size_t i = 0; i < count; ++i) {
        check_result(reqs.reqs[i].data().get(), expected_message, expected_error_code, expected_status);
    }

    reqs.cancel_all();
}

CATCH_TEST_CASE(
    "Test multiple parallel requests against localhost web-server",
    "[http][requests][200][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    perform_parallel_requests(10, url, "URL found");
}

CATCH_TEST_CASE(
    "Test multiple parallel requests against localhost web-server for a non-existing URL",
    "[http][requests][404][localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    perform_parallel_requests(
        10, url, "URL not found",
        http::HTTP_REQUEST_FINISHED,
        http::HTTP_404_NOT_FOUND
    );
}

static void perform_parallel_stream_requests(
    const int count,
    const http::url& url,
    const http::status expected_status = http::HTTP_200_OK
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
    auto url = LOCALHOST + "get_request";
    perform_parallel_stream_requests(10, url);
}

CATCH_TEST_CASE(
    "Test multiple parallel streaming requests against google server",
    "[http][requests][stream][google]"
) {
    auto url = "https://www.google.com/?gws_rd=cr#q=Autodesk";
    perform_parallel_stream_requests(10, url);
}


CATCH_TEST_CASE(
    "Check the default value for the 'connect timeout'",
    "[http][client][connect][timeout][default]"
) {
    CATCH_CHECK(http::client().connect_timeout == 300);
}

CATCH_TEST_CASE(
    "Check the default value for the 'request timeout'",
    "[http][client][request][timeout][default]"
) {
    CATCH_CHECK(http::client().request_timeout == 0);
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
