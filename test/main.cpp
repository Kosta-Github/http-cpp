#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

#include <http-cpp/client.hpp>

int main(int const argc, char* const argv[]) {
    std::wcout.precision(17);
    std::wcerr.precision(17);

    auto res = Catch::Session().run(argc, argv);

    http::client::cancel_all();

    return res;
}
