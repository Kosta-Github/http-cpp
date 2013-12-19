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
#include <http-cpp/utils.hpp>

#include <atomic>

#include <cstdlib>
#include <fstream>

#if defined(NODE_EXE) && defined(NODE_SERVER_JS)

static std::shared_ptr<std::thread> start_node_server() {
    const std::string node_base_url      = "localhost:8888/";
    const std::string node_start_request = node_base_url + "start";
    const std::string node_stop_request  = node_base_url + "stop";

    std::string command = NODE_EXE " \"" NODE_SERVER_JS "\"";
#if defined(WIN32)
    std::replace(command.begin(), command.end(), '/', '\\');
#endif // defined(WIN32)

    auto start_node = [=]() {
        auto res = std::system(command.c_str());
        std::cout << "server exited with code: " << res << std::endl;
    };

    auto stop_node = [=](std::thread* t) {
        assert(t);

        // send the stop request to the server
        http::client().request(node_stop_request).data().wait();

        // wait for the thread to terminate and delete
        // the corresponding object
        t->join();
        delete t;
    };

    auto result = std::shared_ptr<std::thread>(
        new std::thread(start_node), stop_node
    );

    // send the start request to the server and wait for an answer
    while(true) {
        auto&& data = http::client().request(node_start_request).data().get();
        if(data.error_code == http::HTTP_REQUEST_FINISHED) {
            if(data.status == http::HTTP_200_OK) {
                break;
            }
        }
    }

    return result;
}

CATCH_TEST_CASE(
    "Start node web-service",
    "[http][node-server][start]"
) {
    auto node = start_node_server();
};

#endif // defined(NODE_EXE) && defined(NODE_SERVER_JS)


static void print_message(http::message const& msg, std::string const& prefix) {
    std::cout << prefix << "error_code: " << msg.error_code << "\t" << http::error_code_to_string(msg.error_code) << std::endl;
    std::cout << prefix << "status:     " << msg.status << "\t" << http::status_to_string(msg.status) << std::endl;
    std::cout << prefix << "headers: "    << std::endl;
    for(auto&& i : msg.headers) {
        std::cout << prefix << "\t" << i.first << ": " << i.second << std::endl;
    }
    std::cout << prefix << "body:       " << msg.body.size() << std::endl << std::string(msg.body.begin(), msg.body.begin() + std::min(size_t(80), msg.body.size())) << std::endl;
}

static void print_progress(http::progress_info const& progress, std::string const& prefix) {
    std::cout << prefix << "download:   " << progress.downloadCurrentBytes << "/" << progress.downloadTotalBytes << "/" << progress.downloadSpeed << std::endl;
    std::cout << prefix << "upload:     " << progress.uploadCurrentBytes   << "/" << progress.uploadTotalBytes   << "/" << progress.uploadSpeed   << std::endl;
}

static void print_response_data(http::response& res) {
    std::cout << "result:" << std::endl;
    std::cout << "\toperation:  " << http::operation_to_string(res.operation()) << std::endl;
    std::cout << "\trequest:    " << res.request() << std::endl;
    print_progress(res.progress(), "\t");
    print_message(res.data().get(), "\t");
    std::cout << std::endl;
}


/*
CATCH_TEST_CASE(
    "Tests against google web-server",
    "[http][requests][google-host]"
) {
    auto client = http::client();

    auto request = http::request("http://google.com");

    try {
        auto getResponse1       = client.request(request, http::HTTP_GET);
        auto headResponse1      = client.request(request, http::HTTP_HEAD);
        auto postResponse1      = client.request(request, http::HTTP_POST);
        auto putResponse1       = client.request(request, http::HTTP_PUT);
        auto deleteResponse1    = client.request(request, http::HTTP_DELETE);

        auto result = getResponse1.data().wait_for(std::chrono::seconds(1));
        switch(result) {
            case std::future_status::ready:     std::cout << "waiting: ready"       << std::endl; break;
            case std::future_status::timeout:   std::cout << "waiting: timeout"     << std::endl; break;
            case std::future_status::deferred:  std::cout << "waiting: deferred"    << std::endl; break;
            default:                            std::cout << "waiting: unknown"     << std::endl; break;
        }

        deleteResponse1.cancel();

        print_response_data(getResponse1);
        print_response_data(headResponse1);
        print_response_data(postResponse1);
        print_response_data(putResponse1);
        print_response_data(deleteResponse1);

    } catch(std::exception const& ex) {
        std::cerr << "exception: " << ex.what() << " [" << typeid(ex).name() << "]" << std::endl;
    } catch(...) {
        std::cerr << "exception: <unknown type>" << std::endl;
    }
}
*/

CATCH_TEST_CASE(
    "Tests against localhost web-server",
    "[http][requests][local-host]"
) {
    auto client = http::client();

    auto request = http::request("http://google.com");
//    auto request = http::request("http://127.0.0.1:5984/_utils/document.html?kosta/dae10e4df489fa6adacd7efe5e00257a");

    std::atomic<int> active_count(0);

    for(size_t i = 0; i < 10; ++i) {
        ++active_count;
        auto cb = [&, i](http::message msg, http::progress_info progress) -> bool {
            std::cout << "received: " << i << "\t" << http::error_code_to_string(msg.error_code) << "\t" << http::status_to_string(msg.status) << std::endl;
//            print_progress(progress, "\t");
//            print_message(msg, "\t");
//            std::cout << std::endl;

            if(msg.error_code != http::HTTP_REQUEST_PROGRESS) {
                --active_count;
            }

            return true;
        };
        
        client.request_stream(cb, request, http::HTTP_GET);
    }

{ //    while(active_count > 0) {
//        std::cout << "sleeping: " << active_count << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    http::client::cancel_all();
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
