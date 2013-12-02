#include <catch/catch.hpp>

#include <http-cpp/client.hpp>

CATCH_TEST_CASE(
    "dummy",
    "dummy"
) {

std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
    auto client = http::client();
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
    
    auto request = http::request("www.google.com");
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;

    try {
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
        auto getResponse1       = client.request(http::HTTP_GET,    request);
        auto headResponse1      = client.request(http::HTTP_HEAD,   request);
        auto postResponse1      = client.request(http::HTTP_POST,   request);
        auto putResponse1       = client.request(http::HTTP_PUT,    request);
        auto deleteResponse1    = client.request(http::HTTP_DELETE, request);

std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
        auto result = getResponse1.status().wait_for(std::chrono::seconds(1));
        switch(result) {
            case std::future_status::ready:     std::cout << "waiting: ready"       << std::endl; break;
            case std::future_status::timeout:   std::cout << "waiting: timeout"     << std::endl; break;
            case std::future_status::deferred:  std::cout << "waiting: deferred"    << std::endl; break;
            default:                            std::cout << "waiting: unknown"     << std::endl; break;
        }
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
        getResponse1.cancel();
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
    } catch(std::exception const& ex) {
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
        std::cerr << "exception: " << ex.what() << " [" << typeid(ex).name() << "]" << std::endl;
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
    } catch(...) {
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
        std::cerr << "exception: <unknown type>" << std::endl;
std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
    }

std::cout << __FUNCTION__ << ": " << __LINE__ << std::endl;
}
