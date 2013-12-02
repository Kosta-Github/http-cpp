#include "client.hpp"

#include <curl/curl.h>

#include <atomic>

struct http::response::impl {
    std::string source;
    std::string target;
    std::future<http::status> status;
    std::future<http::headers> headers;
    std::future<http::buffer> body;
    std::atomic<bool> cancel;
};

http::response::response() : m_impl(new impl()) { }
http::response::response(response&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::response& http::response::operator=(response&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::response::~response() { delete m_impl; }

std::string const& http::response::source() const { return m_impl->source; }
std::string const& http::response::target() const { return m_impl->target; }
std::future<http::status>& http::response::status() { return m_impl->status; }
std::future<http::headers>& http::response::headers() { return m_impl->headers; }
std::future<http::buffer>& http::response::body() { return m_impl->body; }
void http::response::cancel() { m_impl->cancel = true; }


struct http::client::impl {
    impl() : m_curl(curl_easy_init()) { }
    ~impl() { if(m_curl) { curl_easy_cleanup(m_curl); } }

    CURL* m_curl;
};

http::client::client() :
    m_impl(nullptr)
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    m_impl = new impl();
    
    curl_easy_setopt(m_impl->m_curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(m_impl->m_curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(m_impl->m_curl, CURLOPT_FOLLOWLOCATION, 1);
}
    
http::client::client(client&& o) HTTP_CPP_NOEXCEPT : m_impl(nullptr) { std::swap(m_impl, o.m_impl); }
http::client& http::client::operator=(client&& o) HTTP_CPP_NOEXCEPT { std::swap(m_impl, o.m_impl); return *this; }
http::client::~client() {delete m_impl; }
    
http::response http::client::request(
    http::operation op,
    http::request req,
    receive_body_cb receive_cb,
    http::buffer send_body,
    std::string send_content_type
) {
    std::string url = "http://";
    auto url_escaped = curl_easy_escape(m_impl->m_curl, req.c_str(), 0);
    url += url_escaped;
    curl_free(url_escaped);
    
    curl_easy_setopt(m_impl->m_curl, CURLOPT_URL, url.c_str());
    curl_easy_perform(m_impl->m_curl);

    auto response = http::response();
    
    response.m_impl->status = std::promise<http::status>().get_future();
    response.m_impl->headers = std::promise<http::headers>().get_future();
    response.m_impl->body = std::promise<http::buffer>().get_future();
    
    return response;
}
