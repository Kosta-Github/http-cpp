#include <catch/catch.hpp>

#include <http-cpp/client.hpp>

static void print_response_data(const http::response::info& data) {
    std::cout << "result:" << std::endl;
    std::cout << "\terror_code: " << data.error_code << std::endl;
    std::cout << "\tstatus:     " << data.status << "\t" << http::to_string(data.status) << std::endl;
    std::cout << "\theaders: "    << std::endl;
    for(auto&& i : data.headers) {
        std::cout << "\t\t" << i.first << ": " << i.second << std::endl;
    }
    std::cout << "\tbody:       " << data.body.size() << std::endl << std::string(data.body.begin(), data.body.begin() + std::min(size_t(80), data.body.size())) << std::endl << std::endl;
}



CATCH_TEST_CASE(
    "dummy",
    "dummy"
) {
    auto client = http::client();
    
    auto request = http::request("https://www.google.com");

    try {
        auto getResponse1       = client.request(http::HTTP_GET,    request);
        auto headResponse1      = client.request(http::HTTP_HEAD,   request);
        auto postResponse1      = client.request(http::HTTP_POST,   request);
        auto putResponse1       = client.request(http::HTTP_PUT,    request);
        auto deleteResponse1    = client.request(http::HTTP_DELETE, request);

        auto result = getResponse1.data().wait_for(std::chrono::seconds(1));
        switch(result) {
            case std::future_status::ready:     std::cout << "waiting: ready"       << std::endl; break;
            case std::future_status::timeout:   std::cout << "waiting: timeout"     << std::endl; break;
            case std::future_status::deferred:  std::cout << "waiting: deferred"    << std::endl; break;
            default:                            std::cout << "waiting: unknown"     << std::endl; break;
        }

        deleteResponse1.cancel();

        print_response_data(getResponse1.data().get());
        print_response_data(headResponse1.data().get());
        print_response_data(postResponse1.data().get());
        print_response_data(putResponse1.data().get());
        print_response_data(deleteResponse1.data().get());

    } catch(std::exception const& ex) {
        std::cerr << "exception: " << ex.what() << " [" << typeid(ex).name() << "]" << std::endl;
    } catch(...) {
        std::cerr << "exception: <unknown type>" << std::endl;
    }
}
