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


    struct curl_global_init_wrap {
        curl_global_init_wrap()  { curl_global_init(CURL_GLOBAL_ALL); }
        ~curl_global_init_wrap() { curl_global_cleanup(); }
    };

    struct curl_easy_wrap {
        curl_easy_wrap(CURL* master = nullptr) :
            handle(master ? curl_easy_duphandle(master) : curl_easy_init())
        {
            assert(handle);

            if(!master) { set_default_values(); }
            set_callbacks();
        }

        virtual ~curl_easy_wrap() {
            assert(handle);
            curl_easy_cleanup(handle);
        }

    public:
        void set_default_values() {
            // curl_easy_setopt(global_easy.handle, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(handle, CURLOPT_AUTOREFERER, 1);
            curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, ""); // accept all supported encodings
            curl_easy_setopt(handle, CURLOPT_HTTP_CONTENT_DECODING, 1);
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5); // max 5 redirects
            curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1);
        }

        void set_callbacks() {
            curl_easy_setopt(handle, CURLOPT_PRIVATE, this);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_stub);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(handle, CURLOPT_READFUNCTION, write_stub);
            curl_easy_setopt(handle, CURLOPT_READDATA, this);
            // curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, progress_stub);
            // curl_easy_setopt(handle, CURLOPT_XFERINFODATA, this);
            curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress_stub);
            curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_stub);
            curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);
        }

    public:
        virtual bool   write(const char* ptr, size_t bytes) = 0;
        virtual size_t read(char* ptr, size_t bytes) = 0;
        virtual bool   progress(size_t downTotal, size_t downCur, size_t downSpeed, size_t upTotal, size_t upCur, size_t upSpeed) = 0;
        virtual void   header(const char* ptr, size_t bytes) = 0;
        virtual void   finish(CURLcode code, long status) = 0;

    private:
        static size_t write_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            return (wrap->write(ptr, bytes) ? bytes : 0);
        }

        static  size_t read_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            return wrap->read(ptr, bytes);
        }

        // static  int  progress_function_stub(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        //    auto wrap = static_cast<curl_easy_wrap*>(clientp); assert(wrap);
        //    return (wrap->progress_function(dltotal, dlnow, ultotal, ulnow) ? 0 : 1);
        // }
        static  int  progress_stub(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
            auto wrap = static_cast<curl_easy_wrap*>(clientp); assert(wrap);

            double speedDown = 0.0, speedUp = 0.0;
            curl_easy_getinfo(wrap->handle, CURLINFO_SPEED_DOWNLOAD, &speedDown);
            curl_easy_getinfo(wrap->handle, CURLINFO_SPEED_UPLOAD,   &speedUp);

            auto res = wrap->progress(
                static_cast<size_t>(dltotal), static_cast<size_t>(dlnow), static_cast<size_t>(speedDown),
                static_cast<size_t>(ultotal), static_cast<size_t>(ulnow), static_cast<size_t>(speedUp)
            );
            return (res ? 0 : 1);
        }

        static size_t header_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
            auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
            auto bytes = size * nmemb;
            wrap->header(ptr, bytes);
            return bytes;
        }

    public:
        CURL* const handle;
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
        void add(curl_easy_wrap* wrap) {
            assert(wrap);

            auto handle = wrap->handle;
            assert(handle);

            std::lock_guard<std::mutex> lock(m_mutex);
            assert(m_active_handles[handle] == nullptr);
            m_active_handles[handle] = wrap;

            curl_multi_add_handle(m_multi, handle);
        }

        void remove(curl_easy_wrap* wrap) {
            assert(wrap);

            auto handle = wrap->handle;
            assert(handle);

            std::lock_guard<std::mutex> lock(m_mutex);
            assert(!m_active_handles[handle] || (m_active_handles[handle] == wrap));
            if(m_active_handles.erase(handle)) {
                curl_multi_remove_handle(m_multi, handle);
            }
        }

    private:
        void loop() {
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

                                long status = http::HTTP_000_UNKNOWN;
                                curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status);

                                update_handles.emplace_back([=]() { wrap->finish(error, status); });
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

        bool loop_stop() {
            if(!m_worker_shutdown) { return false; }

            std::lock_guard<std::mutex> lock(m_mutex);
            return m_active_handles.empty();
        }

    private:
        mutable std::mutex m_mutex;
        CURLM* const m_multi;

        std::map<CURL*, curl_easy_wrap*> m_active_handles;

        std::thread         m_worker;
        std::atomic<bool>   m_worker_shutdown;
    };
    
    struct global_data {
        void add(curl_easy_wrap* wrap) {
            assert(wrap);
            m_share.add(wrap->handle);
            m_multi.add(wrap);
        }

        void remove(curl_easy_wrap* wrap) {
            assert(wrap);
            m_multi.remove(wrap);
            m_share.remove(wrap->handle);
        }

    public:
        curl_global_init_wrap m_init;
        curl_share_wrap       m_share;
        curl_multi_wrap       m_multi;
    };

    static global_data& global() {
        static global_data data;
        return data;
    }

} // namespace

struct http::response::impl :
    public curl_easy_wrap
{
    impl(
         http::operation op,
         http::request req,
         CURL* master =  nullptr
    ) :
        curl_easy_wrap(master),
        data_future(data_promise.get_future()),
        finished_future(finished_promise.get_future()),
        m_op(op),
        m_req(std::move(req)),
        progressDownCur(0), progressDownTotal(0), speedDown(0),
        progressUpCur(0),   progressUpTotal(0),   speedUp(0),
        cancel(false),
        send_progress(0)
    {
        data_collected.error_code   = CURLE_OK;
        data_collected.status       = http::HTTP_000_UNKNOWN;

        curl_easy_setopt(handle, CURLOPT_URL, m_req.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY, 0);
    }

    virtual ~impl() {
        finished_future.wait();
        global().remove(this);
    }

    virtual void perform() {
        global().add(this);
    }

    std::promise<http::response::info>  data_promise;
    std::future<http::response::info>   data_future;
    http::response::info                data_collected;

    std::promise<void>  finished_promise;
    std::future<void>   finished_future;

    http::operation m_op;
    http::request   m_req;

    std::atomic<size_t> progressDownCur,    progressDownTotal,  speedDown;
    std::atomic<size_t> progressUpCur,      progressUpTotal,    speedUp;

    std::atomic<bool>   cancel;

    virtual bool write(const char* ptr, size_t bytes) override {
//        std::cout << this << ": write: " << ptr << " (" << bytes << " bytes): " << std::string(ptr, ptr + std::min(bytes, size_t(40))) << std::endl;
        data_collected.body.insert(data_collected.body.end(), ptr, ptr + bytes);
        return true;
    }

    virtual size_t read(char* ptr, size_t bytes) override {
        auto send_bytes = std::min(bytes, send_buffer.size() - send_progress);
        std::memcpy(ptr, send_buffer.data() + send_progress, send_bytes);
        send_progress += send_bytes;
        std::cout << this << ": read: " << ptr << " (" << bytes << " bytes; sending: " << send_bytes << "; remaining: " << (send_buffer.size() - send_progress) << ")" << std::endl;
        return send_bytes;
    }

    virtual bool progress(
        size_t downTotal, size_t downCur, size_t downSpeed,
        size_t upTotal,   size_t upCur,   size_t upSpeed
    ) override {
//        std::cout << this << ": progress: down: " << downTotal << "/" << downCur << ", up: " << upTotal << "/" << upCur << std::endl;
        progressDownCur = downCur;  progressDownTotal   = downTotal; speedDown = downSpeed;
        progressUpCur   = upCur;    progressUpTotal     = upTotal;   speedUp   = upSpeed;

        return !cancel;
    }

    virtual void header(const char* ptr, size_t bytes) override {
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

    virtual void finish(CURLcode code, long status) override {
        auto error = static_cast<http::error_code>(code);
        if((error != http::HTTP_REQUEST_OK) && cancel) {
            error = http::HTTP_REQUEST_CANCELED;
        }

        data_collected.error_code   = error;
        data_collected.status       = static_cast<http::status>(status);
        data_promise.set_value(std::move(data_collected));

        finished_promise.set_value();
    }

    void request_get() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        perform();
    }

    void request_head() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
        perform();
    }

    void request_delete() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
        perform();
    }

    void request_put(http::buffer send_body, std::string content_type) {
        send_buffer = std::move(send_body);
        send_progress = 0;

        auto size = static_cast<curl_off_t>(send_buffer.size());

        curl_easy_setopt(handle, CURLOPT_URL, m_req.c_str());
        curl_easy_setopt(handle, CURLOPT_UPLOAD, 1);
        curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, size);
        perform();
    }

    http::buffer send_buffer;
    size_t       send_progress;
};

http::response::response() : m_impl(nullptr) { }
http::response::response(response&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::response& http::response::operator=(response&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::response::~response() { delete m_impl; }

std::future<http::response::info>& http::response::data() { return m_impl->data_future; }
http::operation http::response::operation() { return m_impl->m_op; }
http::request http::response::request() { return m_impl->m_req; }
void http::response::progress(size_t& outDownCur, size_t& outDownTotal, size_t& outUpCur, size_t& outUpTotal) { outDownCur = m_impl->progressDownCur; outDownTotal = m_impl->progressDownTotal; outUpCur = m_impl->progressUpCur; outUpTotal = m_impl->progressUpTotal; }
void http::response::speed(size_t& outDownSpeed, size_t& outUpSpeed) { outDownSpeed = m_impl->speedDown; outUpSpeed = m_impl->speedUp; }
void http::response::cancel() { m_impl->cancel = true; }


struct http::client::impl {
};

http::client::client() : m_impl(new impl()) { }
http::client::client(client&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::client& http::client::operator=(client&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::client::~client() { delete m_impl; }
    
http::response http::client::request(
    http::operation op,
    http::request req,
    http::headers headers,
    http::buffer send_data,
    std::string data_content_type
) {
    auto response = http::response();
    response.m_impl = new http::response::impl(op, std::move(req));

    switch(op) {
        case http::HTTP_GET:    response.m_impl->request_get(); break;
        case http::HTTP_HEAD:   response.m_impl->request_head(); break;
        case http::HTTP_PUT:    response.m_impl->request_put(std::move(send_data), std::move(data_content_type)); break;
        case http::HTTP_DELETE: response.m_impl->request_delete(); break;
        case http::HTTP_POST:
        default:                response.m_impl->finish(CURLE_UNSUPPORTED_PROTOCOL, http::HTTP_000_UNKNOWN); break;
    }

    return response;
}
