#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

#include <http-cpp/http-cpp.hpp>

int main(int const argc, char* const argv[]) {
    std::wcout.precision(17);
    std::wcerr.precision(17);

    return Catch::Session().run(argc, argv);
}
