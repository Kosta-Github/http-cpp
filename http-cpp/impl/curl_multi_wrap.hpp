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

#pragma once

#include <curl/curl.h>

#include <atomic>

namespace http {
    namespace impl {

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

            void wait_for_all() {
                while(true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    std::lock_guard<std::mutex> lock(m_mutex);
                    if(m_active_handles.empty()) { break; }
                }
            }

            void cancel_all() {
                {   // first cancel all active handles
                    std::lock_guard<std::mutex> lock(m_mutex);
                    for(auto&& i : m_active_handles) {
                        i.second->cancel();
                    }
                }

                // then wait for them to finish
                wait_for_all();
            }

        public:
            void add(std::shared_ptr<http::impl::curl_easy_wrap> wrap) {
                assert(wrap);

                auto handle = wrap->handle;
                assert(handle);

                std::lock_guard<std::mutex> lock(m_mutex);
                assert(m_active_handles[handle] == nullptr);
                m_active_handles[handle] = wrap;

                wrap->prepare();

                curl_multi_add_handle(m_multi, handle);
            }

            void remove(std::shared_ptr<http::impl::curl_easy_wrap> wrap) {
                assert(wrap);

                auto handle = wrap->handle;
                assert(handle);

                std::lock_guard<std::mutex> lock(m_mutex);
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
                    int wait_time_ms = 0;

                    {   // perform curl multi operations within the locked mutex
                        std::lock_guard<std::mutex> lock(m_mutex);
                        auto perform_res = curl_multi_perform(m_multi, &running_handles);

                        // check if we should call perform again immediately
                        if(perform_res == CURLM_CALL_MULTI_PERFORM) {
                            wait_time_ms = 0; // no wait
                        } else if(!m_active_handles.empty()) {
                            wait_time_ms = 10; // some handles are still active => use short wait
                        } else {
                            wait_time_ms = 100; // no handles are active right now => use longer wait
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
                    for(auto&& update_cb : update_handles) { update_cb(); }
                    
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
            
            std::map<CURL*, std::shared_ptr<http::impl::curl_easy_wrap>> m_active_handles;
            
            std::thread         m_worker;
            std::atomic<bool>   m_worker_shutdown;
        };

    } // namespace impl
} // namespace http
