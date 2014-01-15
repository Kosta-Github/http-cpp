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

#pragma once

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
                (void)handle; (void)data; (void)access;
                auto wrap = static_cast<curl_share_wrap*>(userptr); assert(wrap);
                wrap->m_mutex.lock();
            }

            static void unlock_function_stub(CURL* handle, curl_lock_data data, void* userptr) {
                (void)handle; (void)data;
                auto wrap = static_cast<curl_share_wrap*>(userptr); assert(wrap);
                wrap->m_mutex.unlock();
            }
            
            mutable std::mutex m_mutex;
            CURLSH* const      m_share;
        };

    } // namespace impl
} // namespace http
