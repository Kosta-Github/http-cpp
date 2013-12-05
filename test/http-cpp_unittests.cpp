#include <catch/catch.hpp>

#include <http-cpp/client.hpp>

/*
#include <cstdlib>
#include <fstream>

static std::thread start_node_js(
    const std::string filename,
    const std::string& script
) {
    {
        std::ofstream f(filename);
        f << script << std::endl;
    }

    std::string command = "\"" NODE_JS_EXE "\" \"" + filename + "\"";
    return std::thread([=]() { std::system(command.c_str()); });
}
*/


static void print_response_data(http::response& res) {
    auto&& data = res.data().get();

    std::cout << "result:" << std::endl;
    std::cout << "\toperation:  " << http::operation_to_string(res.operation()) << std::endl;
    std::cout << "\trequest:    " << res.request() << std::endl;
    std::cout << "\terror_code: " << data.error_code << "\t" << http::error_code_to_string(data.error_code) << std::endl;
    std::cout << "\tstatus:     " << data.status << "\t" << http::status_to_string(data.status) << std::endl;
    std::cout << "\theaders: "    << std::endl;
    for(auto&& i : data.headers) {
        std::cout << "\t\t" << i.first << ": " << i.second << std::endl;
    }

    auto&& progress = res.progress();
    std::cout << "\tdownload:   " << progress.downloadCurrentBytes << "/" << progress.downloadTotalBytes << "/" << progress.downloadSpeed << std::endl;
    std::cout << "\tupload:     " << progress.uploadCurrentBytes   << "/" << progress.uploadTotalBytes   << "/" << progress.uploadSpeed   << std::endl;

    std::cout << "\tbody:       " << data.body.size() << std::endl << std::string(data.body.begin(), data.body.begin() + std::min(size_t(80), data.body.size())) << std::endl << std::endl;
}



CATCH_TEST_CASE(
    "dummy",
    "dummy"
) {
/*
    std::string filename = "node_js_script.js";
    std::string script = ""
        "var http    = require('http');\n"
        "var url     = require('url');\n"
        "//var process = require('process');\n"
        "\n"
        "http.createServer(function (req, res) {\n"
        "   var u = url.parse(req.url).pathname;\n"
        "   res.writeHead(200, {'Content-Type': 'text/plain'});\n"
        "   res.end('Hello Kosta\\n' + u);\n"
        "   if(u == '/exit') { process.exit(); }"
        "}).listen(1337, '127.0.0.1');\n"
        "console.log('Server running at http://127.0.0.1:1337/');\n";

    auto run_node = start_node_js(filename, script);
*/
    auto client = http::client();
    
    auto request = http::request("http://127.0.0.1:5984/");

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
/*
    auto call_exit = client.request(http::HTTP_GET, request + "exit/bla");
    print_response_data(call_exit);

    run_node.join();
*/
}
