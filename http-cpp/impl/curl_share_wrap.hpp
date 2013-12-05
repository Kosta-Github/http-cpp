#include <curl/curl.h>

#include <mutex>

namespace http {
    namespace impl {

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

    } // namespace impl
} // namespace http
