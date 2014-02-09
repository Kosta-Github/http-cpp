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
#include <fstream>

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
    "Test HTTP_200_OK status",
    "[http][request][200][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    check_result(http::client().request(url).data().get(), "URL found");
}

CATCH_TEST_CASE(
    "Test HTTP_404_NOT_FOUND status",
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
    "Test for an empty error_string on HTTP_200_OK status",
    "[http][request][200][localhost][error_string]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    auto reply = http::client().request(url).data().get();
    check_result(reply, "URL found");
    CATCH_CHECK(reply.error_string == "");
}

CATCH_TEST_CASE(
    "Test canceling a running request",
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
    "Test request timeout",
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
    CATCH_CHECK(data.error_string != "");
    CATCH_CHECK(duration < 3); // should be clearly below the 3 second the web server will delay to answer the request
}

CATCH_TEST_CASE(
    "Test a request on a non-open port",
    "[http][request][non-open-port][localhost]"
) {
    auto url = "http://localhost:1/";
    auto data = http::client().request(url).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code > 0);
    CATCH_CHECK(data.error_string != "");
}

CATCH_TEST_CASE(
    "Test a request to a non-existing hostname",
    "[http][request][invalid-hostname]"
) {
    auto url = "http://abc.xyz/";
    auto data = http::client().request(url).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code > 0);
    CATCH_CHECK(data.error_string != "");
}

CATCH_TEST_CASE(
    "Test a GET request",
    "[http][request][GET][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::GET()).data().get(), "GET received: ");
}

CATCH_TEST_CASE(
    "Test a GET request with writing into a receive file",
    "[http][request][GET][file][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto receive_filename = "receive_file.txt";

    auto client = http::client();
    client.receive_file = receive_filename;
    check_result(client.request(url, http::GET()).data().get(), "");

    std::ifstream receive_file(receive_filename, std::ios::binary);
    std::string line; std::getline(receive_file, line);
    CATCH_CHECK(line == "GET received: ");
    receive_file.close();
    std::remove(receive_filename);
}

CATCH_TEST_CASE(
    "Test a HEAD request",
    "[http][request][HEAD][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::HEAD()).data().get(), ""); // the body needs to be empty for a HEAD request
}

CATCH_TEST_CASE(
    "Test a POST request for sending data directly",
    "[http][request][POST][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the POST workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, http::POST()).data().get(), "POST received: " + send_data);
}

CATCH_TEST_CASE(
    "Test a POST request for sending a file",
    "[http][request][POST][file][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the POST workload!");
    auto send_filename = "post_file.txt";

    std::ofstream send_file(send_filename, std::ios::binary);
    send_file << send_data;
    send_file.close();

    auto client = http::client();
    client.send_file = send_filename;
    check_result(client.request(url, http::POST()).data().get(), "POST received: " + send_data);
    
    std::remove(send_filename);
}

CATCH_TEST_CASE(
    "Test a POST request for sending form data",
    "[http][request][POST][localhost]"
) {
    auto url = LOCALHOST + "echo_request";

    auto post_form = http::form_data();
    post_form.emplace_back("library",   "library_http-cpp_library", "binary");
    post_form.emplace_back("age",       "age_42_age",               "int");
    post_form.emplace_back("color",     "color_red_color");

    auto client = http::client();
    client.post_form = post_form;
    auto data = client.request(url, http::POST()).data().get();

    CATCH_CAPTURE(http::error_code_to_string(data.error_code));
    CATCH_CHECK(data.error_code == http::HTTP_REQUEST_FINISHED);

    CATCH_CAPTURE(http::status_to_string(data.status));
    CATCH_CHECK(data.status == http::HTTP_200_OK);

    CATCH_CAPTURE(data.body);
    CATCH_CHECK(contains(data.body, "POST received: "));

    for(auto&& i : post_form) {
        // check form name
        auto find_name = "Content-Disposition: form-data; name=\"" + i.name + "\"";
        CATCH_CAPTURE(find_name);
        CATCH_CHECK(contains(data.body, find_name));

        // check form content
        CATCH_CAPTURE(i.content);
        CATCH_CHECK(contains(data.body, i.content));

        if(!i.type.empty()) {
            // check form type
            auto find_type = "Content-Type: " + i.type;
            CATCH_CAPTURE(find_type);
            CATCH_CHECK(contains(data.body, find_type));
        }
    }
}

CATCH_TEST_CASE(
    "Test a PUT request",
    "[http][request][PUT][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, http::PUT()).data().get(), "PUT received: " + send_data);
}

CATCH_TEST_CASE(
    "Test a PUT request for sending a file",
    "[http][request][PUT][file][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto send_filename = "put_file.txt";

    std::ofstream send_file(send_filename, std::ios::binary);
    send_file << send_data;
    send_file.close();

    auto client = http::client();
    client.send_file = send_filename;
    check_result(client.request(url, http::PUT()).data().get(), "PUT received: " + send_data);

    std::remove(send_filename);
}

CATCH_TEST_CASE(
    "Test a DELETE request",
    "[http][request][DELETE][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::DELETE()).data().get(), "DELETE received: ");
}

// hide this test since node.js seems not to be able to handle custom HTTP requests... :-(
CATCH_TEST_CASE(
    "Test a CUSTOM request",
    "[hide][http][request][CUSTOM][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the CUSTOM workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, "CUSTOM").data().get(), "CUSTOM received: " + send_data);
}

CATCH_TEST_CASE(
    "Test round tripping HTTP header data",
    "[http][request][headers][localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    auto headers_req = http::headers();
    headers_req["http-cpp"] = "is cool";
    headers_req["cool"]     = "is http-cpp";

    auto client = http::client();
    client.headers = headers_req;
    auto data = client.request(url).data().get();

    check_result(data, "headers received");

    auto headers_received = data.headers;
    for(auto&& h : headers_req) {
        CATCH_CHECK(h.second == headers_received[h.first]);
    }
}

CATCH_TEST_CASE(
    "Test the on_debug callback is working",
    "[http][request][200][localhost][debug]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";

    auto debug_string = std::string();
    auto client = http::client();
    client.on_debug = [&](std::string const& msg) { debug_string += msg; };
    check_result(client.request(url).data().get(), "URL found");

    CATCH_CHECK(contains(debug_string, "curl: [TEXT]: "));
    CATCH_CHECK(contains(debug_string, "curl: [HEADER_IN]: "));
    CATCH_CHECK(contains(debug_string, "curl: [HEADER_OUT]: "));
    CATCH_CHECK(contains(debug_string, "curl: [DATA_IN]: "));
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
    "Test multiple parallel requests",
    "[http][requests][200][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    perform_parallel_requests(10, url, "URL found");
}

CATCH_TEST_CASE(
    "Test multiple parallel requests for a non-existing URL",
    "[http][requests][404][localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    perform_parallel_requests(
        10, url, "URL not found",
        http::HTTP_REQUEST_FINISHED,
        http::HTTP_404_NOT_FOUND
    );
}

CATCH_TEST_CASE(
    "Test progress info after a GET request",
    "[http][request][GET][progress][localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    auto msg = std::string("URL found");

    auto req = http::client().request(url);
    check_result(req.data().get(), msg);

    auto progress = req.progress();

    // verify download progress
    CATCH_CHECK(progress.downloadCurrentBytes == msg.size());
    CATCH_CHECK(progress.downloadTotalBytes == msg.size());
    CATCH_CHECK(progress.downloadSpeed > 0);

    // nothing was uploaded
    CATCH_CHECK(progress.uploadCurrentBytes == 0);
    CATCH_CHECK(progress.uploadTotalBytes == 0);
    CATCH_CHECK(progress.uploadSpeed == 0);
}

CATCH_TEST_CASE(
    "Test progress info after a PUT request",
    "[http][request][PUT][progress][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto client = http::client();
    client.send_data = send_data;
    auto req = client.request(url, http::PUT());
    check_result(req.data().get(), "PUT received: " + send_data);

    auto progress = req.progress();

    // verify download progress
    CATCH_CHECK(progress.downloadCurrentBytes > 0);
    CATCH_CHECK(progress.downloadTotalBytes >= 0);
    CATCH_CHECK(progress.downloadSpeed > 0);

    // verify upload progress
    CATCH_CHECK(progress.uploadCurrentBytes == send_data.size());
    CATCH_CHECK(progress.uploadTotalBytes == send_data.size());
    CATCH_CHECK(progress.uploadSpeed > 0);
}

static void perform_parallel_stream_requests(
    const int count,
    const http::url& url,
    const http::status expected_status = http::HTTP_200_OK
) {
    auto client = http::client();

    std::atomic<int> active_count(count);

    for(int i = 0; i < count; ++i) {
        client.on_receive = [&](http::message msg, http::progress progress) -> bool {
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

        client.request(url);
    }

    while(active_count > 0) {
        std::this_thread::yield();
    }

    client.cancel_all();
}

CATCH_TEST_CASE(
    "Test multiple parallel streaming requests",
    "[http][requests][stream][localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    perform_parallel_stream_requests(10, url);
}

CATCH_TEST_CASE(
    "Test multiple parallel streaming requests against a google server",
    "[http][requests][stream][google]"
) {
    auto url = "https://www.google.com/?gws_rd=cr#q=Autodesk";
    perform_parallel_stream_requests(10, url);
}


CATCH_TEST_CASE(
    "Check the default value for the 'connect timeout' setting",
    "[http][client][connect][timeout][default]"
) {
    CATCH_CHECK(http::client().connect_timeout == 300);
}

CATCH_TEST_CASE(
    "Check the default value for the 'request timeout' setting",
    "[http][client][request][timeout][default]"
) {
    CATCH_CHECK(http::client().request_timeout == 0);
}

CATCH_TEST_CASE(
    "Test http::escape() works properly",
    "[http][escape]"
) {
    CATCH_CHECK(http::escape("hello world") == "hello%20world");
    CATCH_CHECK(http::escape("<>&?%/\\:=*") == "%3C%3E%26%3F%25%2F%5C%3A%3D%2A");
}

CATCH_TEST_CASE(
    "Test http::unescape() works properly",
    "[http][unescape]"
) {
    CATCH_CHECK(http::unescape("hello%20world") == "hello world");
    CATCH_CHECK(http::unescape("%3C%3E%26%3F%25%2F%5C%3A%3D%2A") == "<>&?%/\\:=*");
}
