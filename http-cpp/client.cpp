//
// The MIT License (MIT)
//
// Copyright (c) 2013 by Konstantin (Kosta) Baumann & Autodesk Inc.
//
// Permission is hereby granted, free of charge,  to any person obtaining a copy of
// this software and  associated documentation  files  (the "Software"), to deal in
// the  Software  without  restriction,  including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software,  and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this  permission notice  shall be included in all
// copies or substantial portions of the Software.
//
// THE  SOFTWARE  IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
// IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "client.hpp"
#include "utils.hpp"

#include "impl/curl_easy_wrap.hpp"
#include "impl/curl_global_init_wrap.hpp"
#include "impl/curl_multi_wrap.hpp"
#include "impl/curl_share_wrap.hpp"

// for testing purpose onyl
#include <cstring>

#undef min
#undef max

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
    auto escaped = curl_easy_escape(handle, s.c_str(), static_cast<int>(s.size()));
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
    auto unescaped = curl_easy_unescape(handle, s.c_str(), static_cast<int>(s.size()), &unescaped_len);
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
        std::string data_content_type
    ) :
        curl_easy_wrap(),
        m_message_promise(),
        m_message_future(m_message_promise.get_future().share()),
        m_message_accum(http::HTTP_REQUEST_PROGRESS, http::HTTP_000_UNKNOWN),
        finished_future(finished_promise.get_future()),
        m_cancel(false),
        m_request(std::move(req)),
        m_operation(op),
        m_send_headers(std::move(headers)),
        m_send_data(std::move(send_data)),
        m_send_data_content_type(std::move(data_content_type)),
        m_send_progress(0),
        m_progress_mutex(),
        m_progress()
    {
        curl_easy_setopt(handle, CURLOPT_URL, m_request.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY, 0);
    }

    virtual ~impl() {
        finished_future.wait();
    }

public:
    std::promise<http::message>         m_message_promise;
    std::shared_future<http::message>   m_message_future;
    http::message                       m_message_accum;

    std::promise<void>  finished_promise;
    std::future<void>   finished_future;

    std::atomic<bool>   m_cancel;

    http::request   m_request;
    http::operation m_operation;
    http::headers   m_send_headers;
    http::buffer    m_send_data;
    std::string     m_send_data_content_type;
    size_t          m_send_progress;

    std::function<bool(http::message, http::progress)>  m_receive_cb;
    std::function<void()>                               m_continue_cb;

    std::mutex      m_progress_mutex;
    http::progress  m_progress;

public:
    virtual bool write(const char* ptr, size_t bytes) override {
        if(m_cancel) { return true; }

        // add data to the end of the receive/message buffer
        m_message_accum.body.insert(m_message_accum.body.end(), ptr, ptr + bytes);

        if(m_receive_cb) {
            // call the receive callback with the received data immediately
            http::message msg(http::HTTP_REQUEST_PROGRESS, http::HTTP_200_OK, m_message_accum.headers, std::move(m_message_accum.body));
            m_message_accum.body.clear(); // ensure that the receive/message buffer is in a defined state again

            // call the callback and check for cancelation
            auto proceed = m_receive_cb(std::move(msg), progress());
            if(!proceed) { m_cancel = true; }
        }

        return true;
    }

    virtual size_t read(char* ptr, size_t bytes) override {
        assert(m_send_progress <= m_send_data.size());

        auto send_bytes = std::min(bytes, m_send_data.size() - m_send_progress);
        std::memcpy(ptr, m_send_data.data() + m_send_progress, send_bytes);
        m_send_progress += send_bytes;

        return send_bytes;
    }

    virtual bool progress(
        size_t downCur, size_t downTotal, size_t downSpeed,
        size_t upCur,   size_t upTotal,   size_t upSpeed
    ) override {
        {
            std::lock_guard<std::mutex> lock(m_progress_mutex);
            m_progress.downloadCurrentBytes = downCur;
            m_progress.downloadTotalBytes   = downTotal;
            m_progress.downloadSpeed        = downSpeed;
            m_progress.uploadCurrentBytes   = upCur;
            m_progress.uploadTotalBytes     = upTotal;
            m_progress.uploadSpeed          = upSpeed;
        }

        if(m_receive_cb) {
            http::message msg(http::HTTP_REQUEST_PROGRESS, http::HTTP_200_OK, m_message_accum.headers);
            auto proceed = m_receive_cb(std::move(msg), progress());
            if(!proceed) { m_cancel = true; }
        }

        return !m_cancel;
    }

    http::progress progress() {
        std::lock_guard<std::mutex> lock(m_progress_mutex);
        return m_progress;
    }

    virtual void header(const char* ptr, size_t bytes) override {
        // strip off "CRLF" at the end
        if((bytes > 0) && (ptr[bytes-1] == '\n')) { --bytes; }
        if((bytes > 0) && (ptr[bytes-1] == '\r')) { --bytes; }

        // end of headers found
        if(bytes == 0) { return; }

        // try to extract the key-value pair; if found add it to
        // the message's headers object
        auto str = std::string(ptr, ptr + bytes);
        auto pos = str.find(": ");
        if(pos != str.npos) {
            m_message_accum.headers[str.substr(0, pos)] = str.substr(pos + 2);
        }
    }

    virtual void start() {
        // add this to the list of active requests which
        // actually handles the request in the send/receive
        // thread and also ensures that this object gets
        // not destructed until it is finished.
        global().add(shared_from_this());
    }

    virtual void finish(CURLcode code, long status) override {
        auto error = static_cast<http::error_code>(code);
        if((error != http::HTTP_REQUEST_FINISHED) && m_cancel) {
            error = http::HTTP_REQUEST_CANCELED;
        }

        m_message_accum.error_code  = error;
        m_message_accum.status      = static_cast<http::status>(status);

        // call the receive callback a last time with the final
        // error code
        if(m_receive_cb) {
            assert(m_message_accum.body.empty());
            m_receive_cb(m_message_accum, progress());
        }

        // set the final message data so it can be retrieved
        // with the message-future object in the response object
        m_message_promise.set_value(std::move(m_message_accum));

        // call an optional continuation callback for this request
        if(m_continue_cb) {
            m_continue_cb();
        }

        // mark this request as finished and remove it from the
        // active requests list again
        finished_promise.set_value();
        global().remove(shared_from_this());
    }

    virtual void cancel() override {
        m_cancel = true;
    }

    void request() {
        // dispatch the HTTP operation
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

std::shared_future<http::message>& http::response::data() { return m_impl->m_message_future; }
http::operation http::response::operation() const { return m_impl->m_operation; }
http::request http::response::request() const { return m_impl->m_request; }
http::progress http::response::progress() const { return m_impl->progress(); }
void http::response::cancel() { m_impl->cancel(); }


struct http::client::impl { };

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
    return request(
        nullptr, std::move(req), std::move(op),
        std::move(headers), std::move(send_data), std::move(data_content_type)
    );
}

http::response http::client::request(
    std::function<void(http::response response)> continuationWith,
    http::request req,
    http::operation op,
    http::headers headers,
    http::buffer send_data,
    std::string data_content_type
) {
    auto res_impl = std::make_shared<http::response::impl>(
        std::move(req), std::move(op), std::move(headers),
        std::move(send_data), std::move(data_content_type)
    );

    if(continuationWith) {
        res_impl->m_continue_cb = [=]() {
            auto response = http::response();
            response.m_impl = res_impl;
            continuationWith(std::move(response));
        };
    }

    res_impl->request();

    auto response = http::response();
    response.m_impl = res_impl;
    return response;
}

void http::client::request_stream(
    std::function<bool(http::message data, http::progress progress)> receive_cb,
    http::request req,
    http::operation op,
    http::headers headers,
    http::buffer send_data,
    std::string data_content_type
) {
    assert(receive_cb);

    auto res_impl = std::make_shared<http::response::impl>(
        std::move(req), std::move(op), std::move(headers),
        std::move(send_data), std::move(data_content_type)
    );

    res_impl->m_receive_cb = std::move(receive_cb);

    res_impl->request();
}

void http::client::wait_for_all() { global().m_multi.wait_for_all(); }
void http::client::cancel_all() { global().m_multi.cancel_all(); }
