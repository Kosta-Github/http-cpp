//
// The MIT License (MIT)
//
// Copyright (c) 2013-2014 by Konstantin (Kosta) Baumann & Autodesk Inc.
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

#include <cstring>
#include <cstdio>

#if !defined(_WIN32)
#   include <sys/stat.h>
#endif // !defined(_WIN32)

#undef min
#undef max

namespace {

    static inline std::shared_ptr<FILE> open_file(
        std::string const& filename,
        const char* const flags
    ) {
        assert(flags);

        if(filename.empty()) { return nullptr; }

        std::shared_ptr<FILE> file(
            std::fopen(filename.c_str(), flags),
            [](FILE* f) { if(f) { std::fclose(f); } }
        );

        if(!file) {
            throw std::runtime_error("could not open file: " + filename);
        }

        return file;
    }

    static inline int64_t file_size(
        std::string const& filename
    ) {
#if defined(_WIN32)
        WIN32_FILE_ATTRIBUTE_DATA data;
        if(GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &data)) {
            return static_cast<int64_t>((static_cast<uint64_t>(data.nFileSizeHigh) << 32) + static_cast<uint64_t>(data.nFileSizeLow));
        }
#elif defined(__APPLE__)
        struct stat s;
        if(stat(filename.c_str(), &s) == -1) {
            return static_cast<int64_t>(s.st_size);
        }
#else
        struct stat64 s;
        if(stat64(filename.c_str(), &s) == -1) {
            return static_cast<int64_t>(s.st_size);
        }
#endif

        return -1;
    }

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


struct http::request::impl :
    public http::impl::curl_easy_wrap,
    public std::enable_shared_from_this<impl>
{
    impl(
        http::client&   client,
        http::url       url,
        http::operation op
    ) :
        curl_easy_wrap(),
        m_message_promise(),
        m_message_future(m_message_promise.get_future().share()),
        m_message_accum(http::HTTP_REQUEST_PROGRESS, error_buffer, http::HTTP_000_UNKNOWN),
        finished_future(finished_promise.get_future()),
        m_cancel(false),
        m_url(std::move(url)),
        m_operation(op),
        m_send_data_progress(0),
        m_send_file_size(0),
        m_progress_mutex(),
        m_progress()
    {
        curl_easy_setopt(handle, CURLOPT_URL,       m_url.c_str());
        curl_easy_setopt(handle, CURLOPT_NOBODY,    0);

        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT,    client.connect_timeout);
        curl_easy_setopt(handle, CURLOPT_TIMEOUT,           client.request_timeout);

        for(auto&& h : client.headers) {
            add_header(h.first, h.second);
        }

        using std::swap;
        swap(m_send_data,   client.send_data);
        swap(m_post_form,   client.post_form);

        swap(m_on_progress, client.on_progress);
        swap(m_on_receive,  client.on_receive);

        swap(m_on_debug,    client.on_debug);
        if(m_on_debug) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
        }

        auto on_finish = client.on_finish;
        if(on_finish) {
            m_on_finish = [=]() { auto req = http::request(); req.m_impl = shared_from_this(); on_finish(req); };
            client.on_finish = nullptr;
        }
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

    http::url       m_url;
    http::operation m_operation;

    http::buffer    m_send_data;
    int64_t         m_send_data_progress;

    std::shared_ptr<FILE> m_send_file;
    int64_t               m_send_file_size;

    http::form_data m_post_form;

    std::shared_ptr<FILE> m_receive_file;

    std::function<bool(http::message, http::progress)>  m_on_receive;
    std::function<void()>                               m_on_finish;
    std::function<void(std::string const&)>             m_on_debug;

    std::mutex                          m_progress_mutex;
    http::progress                      m_progress;
    std::function<bool(http::progress)> m_on_progress;

public:
    virtual bool write(const void* ptr, size_t bytes) override {
        if(m_cancel) { return true; }

        auto data = static_cast<const char*>(ptr);

        // add data to the end of the receive/message buffer
        m_message_accum.body.insert(m_message_accum.body.end(), data, data + bytes);

        if(m_on_receive) {
            // call the receive callback with the received data immediately
            http::message msg(http::HTTP_REQUEST_PROGRESS, error_buffer, http::HTTP_200_OK, m_message_accum.headers, std::move(m_message_accum.body));
            m_message_accum.body.clear(); // ensure that the receive/message buffer is in a defined state again

            // call the callback and check for cancelation
            auto proceed = m_on_receive(std::move(msg), progress());
            if(!proceed) { m_cancel = true; }
        }

        return true;
    }

    virtual size_t read(void* ptr, size_t bytes) override {
        assert(0 <= m_send_data_progress);
        assert(m_send_data_progress <= static_cast<int64_t>(m_send_data.size()));

        if(m_cancel) { return 0; }

        auto send_bytes = std::min(static_cast<int64_t>(bytes), static_cast<int64_t>(m_send_data.size()) - m_send_data_progress);
        std::memcpy(ptr, m_send_data.data() + m_send_data_progress, send_bytes);
        m_send_data_progress += send_bytes;

        return send_bytes;
    }

    virtual void debug(int type, std::string const& msg) override {
        if(m_on_debug) {
            auto str = std::string("curl: ");
            switch(type) {
                case CURLINFO_HEADER_IN:    str += "[HEADER_IN]:  "; break;
                case CURLINFO_HEADER_OUT:   str += "[HEADER_OUT]: "; break;
                case CURLINFO_DATA_IN:      str += "[DATA_IN]:    "; break;
                case CURLINFO_DATA_OUT:     str += "[DATA_OUT]:   "; break;
                case CURLINFO_TEXT:
                default:                    str += "[TEXT]:       "; break;
            }
            str += msg;
            m_on_debug(str);
        }
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

        if(m_on_receive) {
            http::message msg(http::HTTP_REQUEST_PROGRESS, error_buffer, http::HTTP_200_OK, m_message_accum.headers);
            auto proceed = m_on_receive(std::move(msg), progress());
            if(!proceed) { m_cancel = true; }
        }

        if(m_on_progress) {
            auto proceed = m_on_progress(progress());
            if(!proceed) { m_cancel = true; }
        }

        return !m_cancel;
    }

    virtual bool seek(int64_t offset, int origin) override {
        if(m_send_file) {
            switch(origin) {
                case SEEK_SET:
                case SEEK_CUR:
                case SEEK_END:  return (std::fseek(m_send_file.get(), static_cast<long>(offset), origin) != 0);
                default:        assert(false); return false;
            }
        } else {
            switch(origin) {
                case SEEK_SET:  m_send_data_progress  = offset;                         return true;
                case SEEK_CUR:  m_send_data_progress += offset;                         return true;
                case SEEK_END:  m_send_data_progress  = offset + m_send_data.size();    return true;
                default:        assert(false);                                          return false;
            }
        }
    }

    http::progress progress() {
        std::lock_guard<std::mutex> lock(m_progress_mutex);
        return m_progress;
    }

    virtual void header(const void* ptr, size_t bytes) override {
        auto data = static_cast<const char*>(ptr);

        // strip off "CRLF" at the end
        if((bytes > 0) && (data[bytes-1] == '\n')) { --bytes; }
        if((bytes > 0) && (data[bytes-1] == '\r')) { --bytes; }

        // end of headers found
        if(bytes == 0) { return; }

        // try to extract the key-value pair; if found add it to
        // the message's headers object
        auto str = std::string(data, data + bytes);
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

    virtual void finish(CURLcode code, int status) override {
        auto error = static_cast<http::error_code>(code);
        if((error != http::HTTP_REQUEST_FINISHED) && m_cancel) {
            error = http::HTTP_REQUEST_CANCELED;
        }

        m_message_accum.error_code      = error;
        m_message_accum.error_string    = error_buffer;
        m_message_accum.status          = static_cast<http::status>(status);

        // call the receive callback a last time with the final
        // error code
        if(m_on_receive) {
            assert(m_message_accum.body.empty());
            m_on_receive(m_message_accum, progress());
        }

        m_send_file.reset();
        m_receive_file.reset();

        // set the final message data so it can be retrieved
        // with the message-future object in the response object
        m_message_promise.set_value(std::move(m_message_accum));

        // call an optional continuation callback for this request
        if(m_on_finish) {
            m_on_finish();
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
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);

        // dispatch the HTTP operation
        if(     m_operation == http::GET())     { request_get();        }
        else if(m_operation == http::HEAD())    { request_head();       }
        else if(m_operation == http::PUT())     { request_put();        }
        else if(m_operation == http::POST())    { request_post();       }
        else if(m_operation == http::PATCH())   { request_patch();      }
        else if(m_operation == http::DELETE())  { request_delete();     }
        else                                    { prepare_send_data();  }

        if(m_receive_file) {
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, nullptr);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA,     m_receive_file.get());
        }

        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, m_operation.c_str());

        start();
    }

    void request_get() {
        assert(!m_send_file);
        assert(m_send_data.empty());
        assert(m_post_form.empty());
    }

    void request_head() {
        request_get();
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    }

    void request_delete() {
        request_get();
    }

    void prepare_send_data() {
        assert(m_post_form.empty());

        if(!m_send_data.empty()) {
            assert(!m_send_file);
            curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,  static_cast<curl_off_t>(m_send_data.size()));
            curl_easy_setopt(handle, CURLOPT_UPLOAD,            1);
        }

        if(m_send_file) {
            assert(m_send_data.empty());

            // set the FILE pointer as read data in CURL
            curl_easy_setopt(handle, CURLOPT_READFUNCTION,      nullptr);
            curl_easy_setopt(handle, CURLOPT_READDATA,          m_send_file.get());
            curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,  static_cast<curl_off_t>(m_send_file_size));
            curl_easy_setopt(handle, CURLOPT_UPLOAD,            1);
        }
    }

    void request_put() {
        prepare_send_data();
    }
    
    void request_post() {
        if(m_post_form.empty()) {
            request_put();
        } else {
            assert(m_send_data.empty());
            assert(!m_send_file);

            for(auto&& i : m_post_form) {
                add_form_data(i.name, i.content, i.type);
            }
            curl_easy_setopt(handle, CURLOPT_HTTPPOST, post_data);
        }
    }

    void request_patch() {
        request_put();
    }

};

std::shared_future<http::message>& http::request::data() { return m_impl->m_message_future; }
http::operation http::request::operation() const { return m_impl->m_operation; }
http::url http::request::url() const { return m_impl->m_url; }
http::progress http::request::progress() const { return m_impl->progress(); }
void http::request::cancel() { m_impl->cancel(); }


http::client::client() : connect_timeout(300), request_timeout(0) { }

http::request http::client::request(
    http::url       url,
    http::operation op
) {
    assert(!op.empty());

    // open_file() can throw an exception
    auto send_file_handle       = open_file(send_file,      "rb");
    auto receive_file_handle    = open_file(receive_file,   "wb");

    auto req = http::request();

    req.m_impl = std::make_shared<http::request::impl>(
        *this, std::move(url), std::move(op)
    );

    req.m_impl->m_send_file     = std::move(send_file_handle);
    req.m_impl->m_receive_file  = std::move(receive_file_handle);

    if(req.m_impl->m_send_file) {
        req.m_impl->m_send_file_size = file_size(send_file);
    }

    req.m_impl->request();

    return req;
}

void http::client::wait_for_all() { global().m_multi.wait_for_all(); }
void http::client::cancel_all() { global().m_multi.cancel_all(); }
