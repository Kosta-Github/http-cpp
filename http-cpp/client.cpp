#include "client.hpp"

#include <curl/curl.h>

#include <atomic>
#include <cassert>
#include <thread>

#undef min
#undef max


#include <iostream>



/*
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
*/

namespace {

    struct curl_wrap;


    struct curl_init_wrap {
        curl_init_wrap()  { curl_global_init(CURL_GLOBAL_ALL); }
        ~curl_init_wrap() { curl_global_cleanup(); }
    };

    struct curl_share_wrap {
        curl_share_wrap() :
            m_share(curl_share_init())
        {
            assert(m_share);
            curl_share_setopt(m_share, CURLSHOPT_LOCKFUNC,   lock_function_stub);
            curl_share_setopt(m_share, CURLSHOPT_UNLOCKFUNC, unlock_function_stub);
            curl_share_setopt(m_share, CURLSHOPT_USERDATA,   this);
            curl_share_setopt(m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
            curl_share_setopt(m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
            curl_share_setopt(m_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        }

        ~curl_share_wrap() {
            assert(m_share);
            curl_share_cleanup(m_share);
        }

        void add(CURL* handle) {
            assert(handle);
            curl_easy_setopt(handle, CURLOPT_SHARE, m_share);
        }

        void remove(CURL* handle) {
            assert(handle);
            curl_easy_setopt(handle, CURLOPT_SHARE, nullptr);
        }

    private:
        static void lock_function_stub(CURL* handle, curl_lock_data data, curl_lock_access access, void* userptr) {
            auto wrap = static_cast<curl_share_wrap*>(userptr); assert(wrap);
            wrap->m_mutex.lock();
        }

        static void unlock_function_stub(CURL* handle, curl_lock_data data, void* userptr) {
            auto wrap = static_cast<curl_share_wrap*>(userptr); assert(wrap);
            wrap->m_mutex.unlock();
        }

        mutable std::mutex m_mutex;
        CURLSH* const      m_share;
    };

    struct curl_multi_wrap {
        curl_multi_wrap() :
            m_multi(curl_multi_init()),
            m_worker_shutdown(false)
        {
            assert(m_multi);

            // start the worker thread
            m_worker = std::thread([this]() { loop(); });
        }

        ~curl_multi_wrap() {
            m_worker_shutdown = true;
            m_worker.join();

            assert(m_active_handles.empty());

            assert(m_multi);
#if defined(WIN32) && defined(NDEBUG)
            // this cleanup call seems to be defect on Windows in debug mode => ignore it for now
            if(m_multi) { curl_multi_cleanup(m_multi); }
#endif // defined(WIN32) && defined(NDEBUG)
        }

    public:
        void add_handle(CURL* handle, curl_wrap* wrap) {
            assert(handle);
            assert(wrap);

            std::lock_guard<std::mutex> lock(m_mutex);
            assert(m_active_handles[handle] == nullptr);
            m_active_handles[handle] = wrap;

            curl_multi_add_handle(m_multi, handle);
        }

        void remove_handle(CURL* handle, curl_wrap* wrap) {
            assert(handle);
            assert(wrap);

            std::lock_guard<std::mutex> lock(m_mutex);
            assert(!m_active_handles[handle] || (m_active_handles[handle] == wrap));
            if(m_active_handles.erase(handle)) {
                curl_multi_remove_handle(m_multi, handle);
            }
        }

    private:
        void loop();

        bool loop_stop() {
            if(!m_worker_shutdown) { return false; }

            std::lock_guard<std::mutex> lock(m_mutex);
            return m_active_handles.empty();
        }

    private:
        mutable std::mutex m_mutex;
        CURLM* const m_multi;

        std::map<CURL*, curl_wrap*> m_active_handles;

        std::thread         m_worker;
        std::atomic<bool>   m_worker_shutdown;
    };
    
    struct global_data {
        global_data() :
            m_init(),
            m_share(),
            m_multi(),
            global_curl(curl_easy_init())
        {
            assert(global_curl);

            // initialize a global curl handle with some default values
            // all other curl handles will clone from this to inherit
            // the same defaults
            curl_easy_setopt(global_curl, CURLOPT_AUTOREFERER, 1);
            curl_easy_setopt(global_curl, CURLOPT_ACCEPT_ENCODING, ""); // accept all supported encodings
            curl_easy_setopt(global_curl, CURLOPT_HTTP_CONTENT_DECODING, 1);
            curl_easy_setopt(global_curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(global_curl, CURLOPT_MAXREDIRS, 5); // max 5 redirects
            // curl_easy_setopt(global_curl, CURLOPT_VERBOSE, 1);
        }

        ~global_data() {
            assert(global_curl);
            curl_easy_cleanup(global_curl);
        }

    public:
        void add_handle(CURL* handle, curl_wrap* wrap) {
            m_share.add(handle);
            m_multi.add_handle(handle, wrap);
        }

        void remove_handle(CURL* handle, curl_wrap* wrap) {
            m_multi.remove_handle(handle, wrap);
            m_share.remove(handle);
        }

    public:
        curl_init_wrap  m_init;
        curl_share_wrap m_share;
        curl_multi_wrap m_multi;
        CURL* const global_curl;
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

            curl_easy_setopt(curl, CURLOPT_PRIVATE, this);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function_stub);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, write_function_stub);
            curl_easy_setopt(curl, CURLOPT_READDATA, this);
            // curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_function_stub);
            // curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_function_stub);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
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
        // static  int  progress_function_stub(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        //    auto wrap = static_cast<curl_wrap*>(clientp); assert(wrap);
        //    return (wrap->progress_function(dltotal, dlnow, ultotal, ulnow) ? 0 : 1);
        // }
        static  int  progress_function_stub(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
            auto wrap = static_cast<curl_wrap*>(clientp); assert(wrap);
            return (wrap->progress_function(static_cast<size_t>(dltotal), static_cast<size_t>(dlnow), static_cast<size_t>(ultotal), static_cast<size_t>(ulnow)) ? 0 : 1);
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

    void curl_multi_wrap::loop() {
        std::vector<std::function<void()>> update_handles;
        while(!loop_stop()) {
            update_handles.clear();
            int running_handles = 0;
            int mesages_left = 0;
            auto wait_time_ms = 10;

            {   // perform curl multi operations within the locked mutex
                std::lock_guard<std::mutex> lock(m_mutex);
                auto perform_res = curl_multi_perform(m_multi, &running_handles);

                // check if we should call perform again immediately
                if(perform_res == CURLM_CALL_MULTI_PERFORM) {
                    wait_time_ms = 0;
                }

                // retrieve the list of handles to update their state
                while(auto msg = curl_multi_info_read(m_multi, &mesages_left)) {
                    auto handle = msg->easy_handle;

                    switch(msg->msg) {
                        case CURLMSG_DONE: {
                            auto it = m_active_handles.find(handle);
                            assert(it != m_active_handles.end());

                            auto wrap = it->second;
                            auto error = msg->data.result;

                            long status_curl = http::HTTP_000_UNKNOWN;
                            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status_curl);
                            auto status = static_cast<http::status>(status_curl);

                            update_handles.emplace_back(
                                [=]() { wrap->finish_function(error, status); }
                            );
                            break;
                        }
                        default: {
                            assert(!"should not be reached");
                            break;
                        }
                    }
                }
            }

            // update the done handles outside of the mutex
            for(auto&& i : update_handles) { i(); }

            if(wait_time_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
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

        data_collected.error_code   = CURLE_OK;
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
