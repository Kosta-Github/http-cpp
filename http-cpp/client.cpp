#include "client.hpp"

#include <curl/curl.h>

#include <atomic>
#include <thread>

#undef min
#undef max


#include <iostream>




static std::string escape_url(
    CURL* const curl,
    const char* const url
) {
    assert(curl);
    assert(url);
    auto escaped = curl_easy_escape(curl, url, 0);
    auto result = std::string(escaped);
    curl_free(escaped);
    return result;
}

namespace {

    struct curl_wrap;

    struct global_data {
        global_data() :
            global_multi(nullptr),
            global_curl(nullptr),
            worker_shutdown(false)
        {
            std::lock_guard<std::mutex> lock(mutex);

            curl_global_init(CURL_GLOBAL_ALL);

            const_cast<CURLM*>(global_multi) = curl_multi_init();

            // initialize a global curl handle with some default values
            // all other curl handles will clone from this to inherit
            // the same defaults
            const_cast<CURL*>(global_curl) = curl_easy_init();
//            curl_easy_setopt(global_curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(global_curl, CURLOPT_AUTOREFERER, 1);
            curl_easy_setopt(global_curl, CURLOPT_ACCEPT_ENCODING, ""); // accept all supported encodings
            curl_easy_setopt(global_curl, CURLOPT_HTTP_CONTENT_DECODING, 1);
            curl_easy_setopt(global_curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(global_curl, CURLOPT_MAXREDIRS, 5); // max 5 redirects

            // start the worker thread
            worker = std::thread([this]() { loop(); });
        }

        ~global_data() {
            std::lock_guard<std::mutex> lock(mutex);

            worker_shutdown = true;
            worker.join();

            assert(active_handles.empty());

            if(global_curl)  { curl_easy_cleanup(global_curl);   }
#if defined(WIN32) && defined(NDEBUG)
            // this cleanup call seems to be defect on Windows in debug mode => ignore it for now
            if(global_multi) { curl_multi_cleanup(global_multi); }
#endif // defined(WIN32) && defined(NDEBUG)

            curl_global_cleanup();
        }

        void add_handle(CURL* handle, curl_wrap* wrap) {
            assert(handle);
            assert(wrap);

            std::lock_guard<std::mutex> lock(mutex);
            assert(active_handles[handle] == nullptr);
            active_handles[handle] = wrap;

            curl_multi_add_handle(global_multi, handle);
        }

        void remove_handle(CURL* handle, curl_wrap* wrap) {
            assert(handle);
            assert(wrap);

            std::lock_guard<std::mutex> lock(mutex);
            if(active_handles.erase(handle)) {
                curl_multi_remove_handle(global_multi, handle);
            }
        }

        void loop();

        bool loop_stop() {
            if(!worker_shutdown) { return false; }

            std::lock_guard<std::mutex> lock(mutex);
            return active_handles.empty();
        }

    public:
        CURL*  const global_curl;
        CURLM* const global_multi;

        mutable std::mutex mutex;
        std::map<CURL*, curl_wrap*> active_handles;

        std::thread         worker;
        std::atomic<bool>   worker_shutdown;
    };

    static global_data& global() {
        static global_data data;
        return data;
    }

    struct curl_wrap {
        curl_wrap(CURL* master) :
            curl(curl_easy_duphandle(master))
        {
            assert(master);
            assert(curl);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function_stub);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, write_function_stub);
            curl_easy_setopt(curl, CURLOPT_READDATA, this);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_function_stub);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_function_stub);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
        }

        virtual ~curl_wrap() {
            global().remove_handle(curl, this);

            curl_easy_cleanup(curl);
        }

        void perform() {
            global().add_handle(curl, this);
        }

        virtual bool   write_function(const char* ptr, size_t bytes) = 0;
        static  size_t write_function_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            return (wrap->write_function(ptr, bytes) ? bytes : 0);
        }

        virtual size_t read_function(const char* ptr, size_t bytes) = 0;
        static  size_t read_function_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            return wrap->read_function(ptr, bytes);
        }

        virtual bool progress_function(size_t downTotal, size_t downCur, size_t upTotal, size_t upCur) = 0;
        static  int  progress_function_stub(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            auto wrap = static_cast<curl_wrap*>(clientp); assert(wrap);
            return (wrap->progress_function(dltotal, dlnow, ultotal, ulnow) ? 0 : 1);
        }

        virtual void  header_function(const char* ptr, size_t bytes) = 0;
        static size_t header_function_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            wrap->header_function(ptr, bytes);
            return bytes;
        }

        virtual void finish_function(http::error_code error, http::status status) = 0;

        CURL* const curl;
    };

    void global_data::loop() {
        while(!loop_stop()) {
            int running_handles = 0;
            auto perform_res = curl_multi_perform(global_multi, &running_handles);

            int mesages_left = 0;
            while(auto msg = curl_multi_info_read(global_multi, &mesages_left)) {
                auto handle = msg->easy_handle;

                switch(msg->msg) {
                    case CURLMSG_DONE: {
                        std::lock_guard<std::mutex> lock(mutex);
                        auto it = active_handles.find(handle);
                        assert(it != active_handles.end());

                        http::error_code error = ((msg->data.result == CURLE_OK) ? http::HTTP_REQUEST_OK : http::HTTP_REQUEST_ERROR);

                        long status = http::HTTP_000_UNKNOWN;
                        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status);

                        it->second->finish_function(error, static_cast<http::status>(status));
                        break;
                    }
                    default: {
                        assert(!"should not be reached");
                        break;
                    }
                }
            }

            if(perform_res == CURLM_CALL_MULTI_PERFORM) {
                std::this_thread::yield();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

} // namespace

struct http::response::impl :
    public curl_wrap
{
    impl(CURL* master) :
        curl_wrap(master),
        data_future(data_promise.get_future()),
        finished_future(finished_promise.get_future()),
        progressDownCur(0), progressDownTotal(0),
        progressUpCur(0),   progressUpTotal(0),
        cancel(false)
    {
        assert(master);
        assert(curl);

        data_collected.error_code   = http::HTTP_REQUEST_ERROR;
        data_collected.status       = http::HTTP_000_UNKNOWN;
    }

    virtual ~impl() {
        finished_future.wait();
    }

    std::promise<http::response::info>  data_promise;
    std::future<http::response::info>   data_future;
    http::response::info                data_collected;

    std::promise<void>  finished_promise;
    std::future<void>   finished_future;

    std::atomic<size_t> progressDownCur,    progressDownTotal;
    std::atomic<size_t> progressUpCur,      progressUpTotal;

    std::atomic<bool>   cancel;

    virtual bool write_function(const char* ptr, size_t bytes) override {
//        std::cout << this << ": write: " << ptr << " (" << bytes << " bytes): " << std::string(ptr, ptr + std::min(bytes, size_t(40))) << std::endl;
        data_collected.body.insert(data_collected.body.end(), ptr, ptr + bytes);
        return true;
    }

    virtual size_t read_function(const char* ptr, size_t bytes) override {
//        std::cout << this << ": read: " << ptr << " (" << bytes << " bytes)" << std::endl;
        return 0;
    }

    virtual bool progress_function(size_t downTotal, size_t downCur, size_t upTotal, size_t upCur) override {
//        std::cout << this << ": progress: down: " << downTotal << "/" << downCur << ", up: " << upTotal << "/" << upCur << std::endl;
        progressDownCur = downCur;  progressDownTotal   = downTotal;
        progressUpCur   = upCur;    progressUpTotal     = upTotal;
        return !cancel;
    }

    virtual void header_function(const char* ptr, size_t bytes) override {
        if(bytes < 2) { return; } // skip invalid data

        // strip off "CRLF" at the end
        if(ptr[bytes-1] == '\n') { --bytes; }
        if(ptr[bytes-1] == '\r') { --bytes; }

        // end of headers found
        if(bytes == 0) { return; }

//        std::cout << this << ": header: " << bytes << " bytes:\n" << std::string(ptr, ptr + bytes) << std::endl;
        auto str = std::string(ptr, ptr + bytes);
        auto pos = str.find(": ");
        if(pos != str.npos) {
            data_collected.headers[str.substr(0, pos)] = str.substr(pos + 2);
        }
    }

    virtual void finish_function(http::error_code error, http::status status) override {
        if((error != http::HTTP_REQUEST_OK) && cancel) {
            error = http::HTTP_REQUEST_CANCELED;
        }

        data_collected.error_code   = error;
        data_collected.status       = status;
        data_promise.set_value(std::move(data_collected));

        finished_promise.set_value();
    }

};

http::response::response() : m_impl(nullptr) { }
http::response::response(response&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::response& http::response::operator=(response&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::response::~response() { delete m_impl; }

std::future<http::response::info>& http::response::data() { return m_impl->data_future; }
void http::response::progress(size_t& outDownCur, size_t& outDownTotal, size_t& outUpCur, size_t& outUpTotal) { outDownCur = m_impl->progressDownCur; outDownTotal = m_impl->progressDownTotal; outUpCur = m_impl->progressUpCur; outUpTotal = m_impl->progressUpTotal; }
void http::response::cancel() { m_impl->cancel = true; }


struct http::client::impl {
    impl(CURL* master) :
        curl(curl_easy_duphandle(master))
    {
        assert(master);
        assert(curl);
    }
    ~impl() {
        curl_easy_cleanup(curl);
    }

    CURL* const curl;
};

http::client::client() : m_impl(new impl(global().global_curl)) { }
http::client::client(client&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::client& http::client::operator=(client&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::client::~client() { delete m_impl; }
    
http::response http::client::request(
    http::operation op,
    http::request req,
    receive_body_cb receive_cb,
    http::buffer send_body,
    std::string send_content_type
) {
    auto response = http::response();
    response.m_impl = new http::response::impl(m_impl->curl);
    auto curl = response.m_impl->curl;
    
    curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

    response.m_impl->perform();

    return response;
}
