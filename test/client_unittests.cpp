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

#include <http-cpp/client.hpp>
#include <http-cpp/requests.hpp>

#include <fstream>

static bool contains(std::string const& str, std::string const& find) {
    return (str.find(find) != str.npos);
}

static inline void check_result(
    http::message const& data,
    std::string const& expected_body,
    http::error_code const expected_error_code = http::HTTP_ERROR_OK,
    http::status const expected_status = http::HTTP_200_OK
) {
    CUTE_ASSERT(data.error_code == expected_error_code, CUTE_CAPTURE(http::to_string(data.error_code)));

    CUTE_ASSERT(data.status == expected_status, CUTE_CAPTURE(http::to_string(data.status)));

    CUTE_ASSERT(data.body == expected_body);
}

static const std::string LOCALHOST = "http://localhost:8888/";

CUTE_TEST(
    "Test HTTP_200_OK status",
    "[http],[request],[200],[localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    check_result(http::client().request(url).data().get(), "URL found");
}

CUTE_TEST(
    "Test HTTP_404_NOT_FOUND status",
    "[http],[request],[404],[localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    check_result(http::client().request(url).data().get(), "URL not found", http::HTTP_ERROR_OK, http::HTTP_404_NOT_FOUND);
}

CUTE_TEST(
    "Test for an empty error_string on HTTP_200_OK status",
    "[http],[request],[200],[localhost],[error_string]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    auto reply = http::client().request(url).data().get();
    check_result(reply, "URL found");
    CUTE_ASSERT(reply.error_string == "");
}

CUTE_TEST(
    "Test canceling a running request",
    "[http],[request],[cancel],[localhost]"
) {
    auto start_time = std::chrono::system_clock::now();

    auto url = LOCALHOST + "delay";
    auto request = http::client().request(url);
    request.cancel(); // cancel the request immediately
    auto data = request.data().get();

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    CUTE_ASSERT(data.error_code == http::HTTP_ERROR_REQUEST_CANCELED, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(duration < 3); // should be clearly below the 3 seconds the web server will delay to answer the request
}

CUTE_TEST(
    "Test that the progress callback gets called during data transfer",
    "[http],[request],[progress],[localhost]"
) {
    std::atomic<size_t> progress_cb_called(0);

    auto url = LOCALHOST + "echo_request";
    auto client = http::client();
    client.on_progress = [&](http::progress) { ++progress_cb_called; return true; };
    auto request = client.request(url);
    auto data = request.data().get();

    CUTE_ASSERT(data.error_code == http::HTTP_ERROR_OK, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(progress_cb_called > 0);
}

CUTE_TEST(
    "Test canceling a running request from within a progress callback",
    "[http],[request],[cancel],[progress],[localhost]"
) {
    std::atomic<bool> progress_cb_called(false);

    auto url = LOCALHOST + "delay";
    auto client = http::client();
    client.on_progress = [&](http::progress) { progress_cb_called = true; return false; };
    auto request = client.request(url);
    auto data = request.data().get();

    CUTE_ASSERT(data.error_code == http::HTTP_ERROR_REQUEST_CANCELED, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(progress_cb_called);
}

CUTE_TEST(
    "Test request timeout",
    "[http],[request],[timeout],[localhost]"
) {
    auto start_time = std::chrono::system_clock::now();

    auto url = LOCALHOST + "delay";
    auto client = http::client();
    client.request_timeout = 1; // 1 second timeout
    auto data = client.request(url).data().get();

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    CUTE_ASSERT(data.error_code > 0, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(data.error_string != "");
    CUTE_ASSERT(duration < 3); // should be clearly below the 3 second the web server will delay to answer the request
}

CUTE_TEST(
    "Test a request on a non-open port",
    "[http],[request],[non-open-port],[localhost]"
) {
    auto url = "http://localhost:1/";
    auto data = http::client().request(url).data().get();

    CUTE_ASSERT(data.error_code > 0, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(data.error_string != "");
}

CUTE_TEST(
    "Test a request to a non-existing hostname",
    "[http],[request],[invalid-hostname]"
) {
    auto url = "http://non-existing.http-cpp/";
    auto data = http::client().request(url).data().get();

    CUTE_ASSERT(data.error_code > 0, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(data.error_string != "");
}

CUTE_TEST(
    "Test a GET request",
    "[http],[request],[GET],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::OP_GET()).data().get(), "GET received: ");
}

CUTE_TEST(
    "Test a GET request with writing into a receive file",
    "[http],[request],[GET],[file],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto receive_filename = cute::temp_folder() + "receive_file.txt";

    auto client = http::client();
    client.receive_file = receive_filename;
    check_result(client.request(url, http::OP_GET()).data().get(), "");

    std::ifstream receive_file(receive_filename, std::ios::binary);
    std::string line; std::getline(receive_file, line);
    CUTE_ASSERT(line == "GET received: ");
}

CUTE_TEST(
    "Test a HEAD request",
    "[http],[request],[HEAD],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::OP_HEAD()).data().get(), ""); // the body needs to be empty for a HEAD request
}

CUTE_TEST(
    "Test a POST request for sending data directly",
    "[http],[request],[POST],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the POST workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, http::OP_POST()).data().get(), "POST received: " + send_data);
}

CUTE_TEST(
    "Test a POST request for sending a file",
    "[http],[request],[POST],[file],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the POST workload!");
    auto send_filename = cute::temp_folder() + "post_file.txt";

    std::ofstream send_file(send_filename, std::ios::binary);
    send_file << send_data;
    send_file.close();

    auto client = http::client();
    client.send_file = send_filename;
    check_result(client.request(url, http::OP_POST()).data().get(), "POST received: " + send_data);
}

CUTE_TEST(
    "Test sending a non-existing file is signaled with an error",
    "[http],[request],[POST],[file],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto not_existing_filename = cute::temp_folder() + "not_existing.txt";

    auto client = http::client();
    client.send_file = not_existing_filename;
    check_result(client.request(url, http::OP_POST()).data().get(), "", http::HTTP_ERROR_COULDNT_OPEN_SEND_FILE, http::HTTP_000_UNKNOWN);
}

CUTE_TEST(
    "Test a POST request for sending form data",
    "[http],[request],[POST],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";

    auto post_form = http::form_data();
    post_form.emplace_back("library",   "library_http-cpp_library", "binary");
    post_form.emplace_back("age",       "age_42_age",               "int");
    post_form.emplace_back("color",     "color_red_color");

    auto client = http::client();
    client.post_form = post_form;
    auto data = client.request(url, http::OP_POST()).data().get();

    CUTE_ASSERT(data.error_code == http::HTTP_ERROR_OK, CUTE_CAPTURE(http::to_string(data.error_code)));
    CUTE_ASSERT(data.status == http::HTTP_200_OK, CUTE_CAPTURE(http::to_string(data.status)));
    CUTE_ASSERT(contains(data.body, "POST received: "), CUTE_CAPTURE(data.body));

    for(auto&& i : post_form) {
        // check form name
        auto find_name = "Content-Disposition: form-data; name=\"" + i.name + "\"";
        CUTE_ASSERT(contains(data.body, find_name), CUTE_CAPTURE(find_name));

        // check form content
        CUTE_ASSERT(contains(data.body, i.content), CUTE_CAPTURE(i.content));

        if(!i.type.empty()) {
            // check form type
            auto find_type = "Content-Type: " + i.type;
            CUTE_ASSERT(contains(data.body, find_type), CUTE_CAPTURE(find_type));
        }
    }
}

CUTE_TEST(
    "Test a PUT request",
    "[http],[request],[PUT],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, http::OP_PUT()).data().get(), "PUT received: " + send_data);
}

CUTE_TEST(
    "Test a PUT request for sending a file",
    "[http],[request],[PUT],[file],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto send_filename = cute::temp_folder() + "put_file.txt";

    std::ofstream send_file(send_filename, std::ios::binary);
    send_file << send_data;
    send_file.close();

    auto client = http::client();
    client.send_file = send_filename;
    check_result(client.request(url, http::OP_PUT()).data().get(), "PUT received: " + send_data);
}

CUTE_TEST(
    "Test a DELETE request",
    "[http],[request],[DELETE],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    check_result(http::client().request(url, http::OP_DELETE()).data().get(), "DELETE received: ");
}

// hide this test since node.js seems not to be able to handle custom HTTP requests... :-(
/*
CUTE_TEST(
    "Test a CUSTOM request",
    "[http],[request],[CUSTOM],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the CUSTOM workload!");
    auto client = http::client();
    client.send_data = send_data;
    check_result(client.request(url, "CUSTOM").data().get(), "CUSTOM received: " + send_data);
}
*/

CUTE_TEST(
    "Test round tripping HTTP header data",
    "[http],[request],[headers],[localhost]"
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
        CUTE_ASSERT(h.second == headers_received[h.first]);
    }
}

CUTE_TEST(
    "Test the on_debug callback is working",
    "[http],[request],[200],[localhost],[debug]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";

    auto debug_string = std::string();
    auto client = http::client();
    client.on_debug = [&](std::string const& msg) { debug_string += msg; };
    check_result(client.request(url).data().get(), "URL found");

    CUTE_ASSERT(contains(debug_string, "curl: [TEXT]: "));
    CUTE_ASSERT(contains(debug_string, "curl: [HEADER_IN]: "));
    CUTE_ASSERT(contains(debug_string, "curl: [HEADER_OUT]: "));
    CUTE_ASSERT(contains(debug_string, "curl: [DATA_IN]: "));
}

static void perform_parallel_requests(
    const size_t count,
    const http::url url,
    const std::string& expected_message,
    const http::error_code expected_error_code = http::HTTP_ERROR_OK,
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

CUTE_TEST(
    "Test multiple parallel requests",
    "[http],[requests],[200],[localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    perform_parallel_requests(10, url, "URL found");
}

CUTE_TEST(
    "Test multiple parallel requests for a non-existing URL",
    "[http],[requests],[404],[localhost]"
) {
    auto url = LOCALHOST + "HTTP_404_NOT_FOUND";
    perform_parallel_requests(
        10, url, "URL not found",
        http::HTTP_ERROR_OK,
        http::HTTP_404_NOT_FOUND
    );
}

CUTE_TEST(
    "Test progress info after a GET request",
    "[http],[request],[GET],[progress],[localhost]"
) {
    auto url = LOCALHOST + "HTTP_200_OK";
    auto msg = std::string("URL found");

    auto req = http::client().request(url);
    check_result(req.data().get(), msg);

    auto progress = req.progress();

    // verify download progress
    CUTE_ASSERT(progress.downloadCurrentBytes == msg.size());
    CUTE_ASSERT(progress.downloadTotalBytes == msg.size());
    CUTE_ASSERT(progress.downloadSpeed > 0);

    // nothing was uploaded
    CUTE_ASSERT(progress.uploadCurrentBytes == 0);
    CUTE_ASSERT(progress.uploadTotalBytes == 0);
    CUTE_ASSERT(progress.uploadSpeed == 0);
}

CUTE_TEST(
    "Test progress info after a PUT request",
    "[http],[request],[PUT],[progress],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    auto send_data = std::string("I am the PUT workload!");
    auto client = http::client();
    client.send_data = send_data;
    auto req = client.request(url, http::OP_PUT());
    check_result(req.data().get(), "PUT received: " + send_data);

    auto progress = req.progress();

    // verify download progress
    CUTE_ASSERT(progress.downloadCurrentBytes > 0);
    // CUTE_ASSERT(progress.downloadTotalBytes >= 0); // is always true
    CUTE_ASSERT(progress.downloadSpeed > 0);

    // verify upload progress
    CUTE_ASSERT(progress.uploadCurrentBytes == send_data.size());
    CUTE_ASSERT(progress.uploadTotalBytes == send_data.size());
    CUTE_ASSERT(progress.uploadSpeed > 0);
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
                case http::HTTP_ERROR_REPORT_PROGRESS: {
                    break;
                }
                case http::HTTP_ERROR_OK: {
                    CUTE_ASSERT(msg.status == expected_status);
                    --active_count;
                    break;
                }
                default: {
                    CUTE_ASSERT(msg.error_code == http::HTTP_ERROR_REPORT_PROGRESS, CUTE_CAPTURE(http::to_string(msg.error_code)));
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

CUTE_TEST(
    "Test multiple parallel streaming requests",
    "[http],[requests],[stream],[localhost]"
) {
    auto url = LOCALHOST + "echo_request";
    perform_parallel_stream_requests(10, url);
}

CUTE_TEST(
    "Test multiple parallel streaming requests against a google server",
    "[http],[requests],[stream],[google]"
) {
    auto url = "https://www.google.com/?gws_rd=cr#q=Autodesk";
    perform_parallel_stream_requests(10, url);
}


CUTE_TEST(
    "Check the default value for the 'connect timeout' setting",
    "[http],[client],[connect],[timeout],[default]"
) {
    CUTE_ASSERT(http::client().connect_timeout == 300);
}

CUTE_TEST(
    "Check the default value for the 'request timeout' setting",
    "[http],[client],[request],[timeout],[default]"
) {
    CUTE_ASSERT(http::client().request_timeout == 0);
}

CUTE_TEST(
    "Test correct handling of accept_compressed=true",
    "[http],[request],[accept_compressed],[localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    auto client = http::client();
    client.accept_compressed = true;
    auto data = client.request(url).data().get();

    check_result(data, "headers received");

    auto headers_received = data.headers;
    CUTE_ASSERT(headers_received.count("accept-encoding") > 0);
}

CUTE_TEST(
    "Test correct handling of accept_compressed=false",
    "[http],[request],[accept_compressed],[localhost]"
) {
    auto url = LOCALHOST + "echo_headers";

    auto client = http::client();
    client.accept_compressed = false;
    auto data = client.request(url).data().get();

    check_result(data, "headers received");

    auto headers_received = data.headers;
    CUTE_ASSERT(headers_received.count("accept-encoding") == 0);
}
