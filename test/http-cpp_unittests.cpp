#include <catch/catch.hpp>

#include <http-cpp/client.hpp>
#include <http-cpp/utils.hpp>

#include <atomic>

#include <cstdlib>
#include <fstream>

#if defined(PYTHON_EXE)

static std::thread start_python(
    const std::string filename,
    const std::string& script
) {
    {
        std::ofstream f(filename);
        f << script << std::endl;
    }

    std::string command = "\"" PYTHON_EXE "\" \"" + filename + "\"";
    return std::thread([=]() { std::system(command.c_str()); });
}

CATCH_TEST_CASE(
    "Start python web-service",
    "[http][python][start]"
) {
     std::string filename = "web-service.py";
     std::string script = ""
        "import web\n"
        "\n"
        "urls = (\n"
        "    '/', 'index'\n"
        ")\n"
        "\n"
        "class index:\n"
        "    def GET(self):\n"
        "        return \"Hello, world!\"\n"
        "\n"
        "if __name__ == \"__main__\":\n"
        "    app = web.application(urls, globals())\n"
        "    app.run()\n";

//     auto run_node = start_python(filename, script);
};

#endif // defined(PYTHON_EXE)


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
