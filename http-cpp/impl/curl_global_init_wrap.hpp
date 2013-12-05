#include <curl/curl.h>

namespace http {
    namespace impl {

        struct curl_global_init_wrap {
            curl_global_init_wrap()  { curl_global_init(CURL_GLOBAL_ALL); }
            ~curl_global_init_wrap() { curl_global_cleanup(); }
        };

    } // namespace impl
} // namespace http
