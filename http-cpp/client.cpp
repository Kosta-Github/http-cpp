#include "client.hpp"
#include "utils.hpp"

#include "impl/curl_easy_wrap.hpp"
#include "impl/curl_global_init_wrap.hpp"
#include "impl/curl_multi_wrap.hpp"
#include "impl/curl_share_wrap.hpp"

#include <curl/curl.h>

#include <atomic>
#include <cassert>
#include <thread>

#include <iostream>

namespace {

    struct global_data {
        global_data() : m_dummy_handle(curl_easy_init()) { }
        ~global_data() { curl_easy_cleanup(m_dummy_handle); }

        void add(std::shared_ptr<http::impl::curl_easy_wrap> wrap) {
            assert(wrap);
            m_share.add(wrap->handle);
            m_multi.add(wrap);
        }

        void remove(std::shared_ptr<http::impl::curl_easy_wrap> wrap) {
            assert(wrap);
            m_multi.remove(wrap);
            m_share.remove(wrap->handle);
        }

    public:
        http::impl::curl_global_init_wrap   m_init;
        http::impl::curl_share_wrap         m_share;
        http::impl::curl_multi_wrap         m_multi;
        CURL* const                         m_dummy_handle; // e.g., for escape()/unescape()
    };

    static global_data& global() {
        static global_data data;
        return data;
    }

} // namespace


std::string http::escape(
    std::string s
) {
    if(s.empty()) { return s; }

    auto handle = global().m_dummy_handle;
    auto escaped = curl_easy_escape(handle, s.c_str(), s.size());
    s = escaped;
    curl_free(escaped);

    return s;
}

std::string http::unescape(
    std::string s
) {
    if(s.empty()) { return s; }

    auto handle = global().m_dummy_handle;
    int unescaped_len = 0;
    auto unescaped = curl_easy_unescape(handle, s.c_str(), s.size(), &unescaped_len);
    s.assign(unescaped, unescaped_len);
    curl_free(unescaped);
    
    return s;
}


struct http::response::impl :
    public http::impl::curl_easy_wrap,
    public std::enable_shared_from_this<impl>
{
    impl(
        http::request req,
        http::operation op,
        http::headers headers,
        http::buffer send_data,
        std::string data_content_type,
        std::function<bool(http::message, http::progress_info)> receive_cb
    ) :
        curl_easy_wrap(),
        data_future(data_promise.get_future()),
        finished_future(finished_promise.get_future()),
        m_request(std::move(req)),
        m_operation(op),
        m_send_headers(std::move(headers)),
        m_send_data(std::move(send_data)),
        m_send_data_content_type(std::move(data_content_type)),
        m_send_progress(0),
        m_receive_cb(std::move(receive_cb)),
        progressDownCur(0), progressDownTotal(0), speedDown(0),
        progressUpCur(0),   progressUpTotal(0),   speedUp(0),
        cancel(false)
    {
        data_collected.error_code   = CURLE_OK;
        data_collected.status       = http::HTTP_000_UNKNOWN;

        curl_easy_setopt(handle, CURLOPT_URL, m_request.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY, 0);
    }

    virtual ~impl() {
        finished_future.wait();
    }

    std::promise<http::message>  data_promise;
    std::future<http::message>   data_future;
    http::message                data_collected;

    std::promise<void>  finished_promise;
    std::future<void>   finished_future;

    http::request   m_request;
    http::operation m_operation;
    http::headers   m_send_headers;
    http::buffer    m_send_data;
    std::string     m_send_data_content_type;
    size_t          m_send_progress;

    std::function<bool(http::message, http::progress_info)> m_receive_cb;

    std::atomic<size_t> progressDownCur,    progressDownTotal,  speedDown;
    std::atomic<size_t> progressUpCur,      progressUpTotal,    speedUp;

    std::atomic<bool>   cancel;

    virtual bool write(const char* ptr, size_t bytes) override {
//        std::cout << this << ": write: " << ptr << " (" << bytes << " bytes): " << std::string(ptr, ptr + std::min(bytes, size_t(40))) << std::endl;
        data_collected.body.insert(data_collected.body.end(), ptr, ptr + bytes);

        if(m_receive_cb) {
            http::message msg(data_collected.error_code, data_collected.status, data_collected.headers, std::move(data_collected.body));
            cancel = !m_receive_cb(std::move(msg), progress());
        } else {
        }

        return true;
    }

    virtual size_t read(char* ptr, size_t bytes) override {
        auto send_bytes = std::min(bytes, m_send_data.size() - m_send_progress);
        std::memcpy(ptr, m_send_data.data() + m_send_progress, send_bytes);
        m_send_progress += send_bytes;
        std::cout << this << ": read: " << ptr << " (" << bytes << " bytes; sending: " << send_bytes << "; remaining: " << (m_send_data.size() - m_send_progress) << ")" << std::endl;
        return send_bytes;
    }

    virtual bool progress(
        size_t downTotal, size_t downCur, size_t downSpeed,
        size_t upTotal,   size_t upCur,   size_t upSpeed
    ) override {
        progressDownCur = downCur;  progressDownTotal   = downTotal; speedDown = downSpeed;
        progressUpCur   = upCur;    progressUpTotal     = upTotal;   speedUp   = upSpeed;

        return !cancel;
    }

    http::progress_info progress() {
        return http::progress_info(progressDownCur, progressDownTotal, speedDown, progressUpCur, progressUpTotal, speedUp);
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

    virtual void start() {
        global().add(shared_from_this());
    }

    virtual void finish(CURLcode code, long status) override {
        auto error = static_cast<http::error_code>(code);
        if((error != http::HTTP_REQUEST_OK) && cancel) {
            error = http::HTTP_REQUEST_CANCELED;
        }

        data_collected.error_code   = error;
        data_collected.status       = static_cast<http::status>(status);

        if(m_receive_cb) {
            assert(data_collected.body.empty());
            m_receive_cb(std::move(data_collected), progress());
        } else {
            data_promise.set_value(std::move(data_collected));
        }

        finished_promise.set_value();

        global().remove(shared_from_this());
    }

    void request() {
        switch(m_operation) {
            case http::HTTP_GET:    request_get();      break;
            case http::HTTP_HEAD:   request_head();     break;
            case http::HTTP_PUT:    request_put();      break;
            case http::HTTP_DELETE: request_delete();   break;
            case http::HTTP_POST:
            default:                finish(CURLE_UNSUPPORTED_PROTOCOL, http::HTTP_000_UNKNOWN); break;
        }
    }

    void request_get() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        start();
    }

    void request_head() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
        start();
    }

    void request_delete() {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
        start();
    }

    void request_put() {
        auto size = static_cast<curl_off_t>(m_send_data.size());

        curl_easy_setopt(handle, CURLOPT_UPLOAD, 1);
        curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, size);
        start();
    }
};

http::response::response() : m_impl(nullptr) { }
http::response::response(response&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::response& http::response::operator=(response&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::response::~response() { }

std::future<http::message>& http::response::data() { return m_impl->data_future; }
http::operation http::response::operation() { return m_impl->m_operation; }
http::request http::response::request() { return m_impl->m_request; }
http::progress_info http::response::progress() { return m_impl->progress(); }
void http::response::cancel() { m_impl->cancel = true; }


struct http::client::impl {
};

http::client::client() : m_impl(new impl()) { }
http::client::client(client&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::client& http::client::operator=(client&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::client::~client() { delete m_impl; }
    
http::response http::client::request(
    http::request req,
    http::operation op,
    http::headers headers,
    http::buffer send_data,
    std::string data_content_type
) {
    auto response = http::response();
    response.m_impl = std::make_shared<http::response::impl>(
        std::move(req), std::move(op), std::move(headers),
        std::move(send_data), std::move(data_content_type),
        nullptr
    );
    response.m_impl->request();
    return response;
}

void http::client::request_stream(
    std::function<bool(http::message data, http::progress_info progress)> receive_cb,
    http::request req,
    http::operation op,
    http::headers headers,
    http::buffer send_data,
    std::string data_content_type
) {
    assert(receive_cb);

    auto response = std::make_shared<http::response::impl>(
        std::move(req), std::move(op), std::move(headers),
        std::move(send_data), std::move(data_content_type),
        receive_cb
    );
    response->request();
}
