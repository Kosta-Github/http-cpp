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

#include <cassert>

namespace http {
    namespace impl {

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
                curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_stub);
                curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);
                // curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, progress_stub);
                // curl_easy_setopt(handle, CURLOPT_XFERINFODATA, this);
                curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress_stub);
                curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this);
                curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
            }

        public:
            virtual bool   write(const char* ptr, size_t bytes) = 0;
            virtual size_t read(char* ptr, size_t bytes) = 0;
            virtual void   header(const char* ptr, size_t bytes) = 0;
            virtual bool   progress(size_t downTotal, size_t downCur, size_t downSpeed, size_t upTotal, size_t upCur, size_t upSpeed) = 0;
            virtual void   finish(CURLcode code, long status) = 0;
            virtual void   cancel() = 0;

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

            static size_t header_stub(char* ptr, size_t size, size_t nmemb, void* userdata) {
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto bytes = size * nmemb;
                wrap->header(ptr, bytes);
                return bytes;
            }

            // static int progress_function_stub(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            static int progress_stub(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
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
            
        public:
            CURL* const handle;
        };

    } // namespace impl
} // namespace http
