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

#include "start_node_server.hpp"

#include <http-cpp/client.hpp>

#include <cassert>
#include <cstdlib>
#include <iostream>

#if defined(NODE_EXE) && defined(NODE_SERVER_JS)

std::shared_ptr<std::thread> start_node_server() {
    const std::string node_base_url      = "localhost:8888/";
    const std::string node_start_request = node_base_url + "start";
    const std::string node_stop_request  = node_base_url + "stop";

    std::string command = "\"\"" NODE_EXE "\" \"" NODE_SERVER_JS "\"\"";
#if defined(WIN32)
    std::replace(command.begin(), command.end(), '/', '\\');
#endif // defined(WIN32)

    auto start_node = [=]() {
        auto res = std::system(command.c_str());
        std::cout << "Local node.js server exited: " << ((res == EXIT_SUCCESS) ? "success" : "failure");
        std::cout << " [code: " << res << "]." << std::endl;
    };

    auto stop_node = [=](std::thread* t) {
        assert(t);

        // send the stop request to the server
        http::client().request(node_stop_request).wait();

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
        if(data.error_code == http::HTTP_ERROR_OK) {
            if(data.status == http::HTTP_200_OK) {
                break;
            }
        }
    }

    return result;
}

#else // defined(NODE_EXE) && defined(NODE_SERVER_JS)

std::shared_ptr<std::thread> start_node_server() {
    std::cerr << "not configured for using node.js as testing server!" << std::endl;

    return nullptr;
}

#endif // defined(NODE_EXE) && defined(NODE_SERVER_JS)
