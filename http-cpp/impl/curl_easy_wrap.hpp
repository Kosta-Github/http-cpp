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

#include <cassert>

namespace http {
    namespace impl {

        struct curl_easy_wrap {
            curl_easy_wrap(CURL* master = nullptr) :
                handle(master ? curl_easy_duphandle(master) : curl_easy_init()),
                headers(nullptr),
                post_data(nullptr),
                post_data_last(nullptr)
            {
                assert(handle);

                error_buffer[0] = 0x00; // ensure a zero-terminating of the error buffer

                if(!master) { set_default_values(); }
                set_callbacks();
            }

            virtual ~curl_easy_wrap() {
                assert(handle);
                curl_easy_cleanup(handle);

                curl_slist_free_all(headers);

                curl_formfree(post_data);
            }

        public:
            void set_default_values() {
//                curl_easy_setopt(handle, CURLOPT_VERBOSE,               1);

                curl_easy_setopt(handle, CURLOPT_AUTOREFERER,           1);
                curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING,       ""); // accept all supported encodings
                curl_easy_setopt(handle, CURLOPT_HTTP_CONTENT_DECODING, 1);
                curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION,        1);
                curl_easy_setopt(handle, CURLOPT_MAXREDIRS,             5); // max 5 redirects
#if (LIBCURL_VERSION_NUM >= 0x071900)
                curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE,         1);
#endif // (LIBCURL_VERSION_NUM >= 0x071900)
                curl_easy_setopt(handle, CURLOPT_ERRORBUFFER,           error_buffer);

                // SSL related defaults
                curl_easy_setopt(handle, CURLOPT_SSLVERSION,            CURL_SSLVERSION_TLSv1); // explicitly disable SSLv3 and below; only allow TLSv1.x
                curl_easy_setopt(handle, CURLOPT_SSL_ENABLE_NPN,        0); // disable NPN support (use ALPN instead); see:http://blog.chromium.org/2015/02/hello-http2-goodbye-spdy-http-is_9.html
            }

            void set_callbacks() {
                curl_easy_setopt(handle, CURLOPT_PRIVATE,           this);
                curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,     write_stub);
                curl_easy_setopt(handle, CURLOPT_WRITEDATA,         this);
                curl_easy_setopt(handle, CURLOPT_READFUNCTION,      read_stub);
                curl_easy_setopt(handle, CURLOPT_READDATA,          this);
                curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION,    header_stub);
                curl_easy_setopt(handle, CURLOPT_HEADERDATA,        this);
                curl_easy_setopt(handle, CURLOPT_SEEKFUNCTION,      seek_stub);
                curl_easy_setopt(handle, CURLOPT_SEEKDATA,          this);
                curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION,     debug_stub);
                curl_easy_setopt(handle, CURLOPT_DEBUGDATA,         this);

#if (LIBCURL_VERSION_NUM >= 0x072000)
                curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION,  progress_stub);
                curl_easy_setopt(handle, CURLOPT_XFERINFODATA,      this);
#else // (LIBCURL_VERSION_NUM >= 0x072000)
                curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION,  progress_stub);
                curl_easy_setopt(handle, CURLOPT_PROGRESSDATA,      this);
#endif // (LIBCURL_VERSION_NUM >= 0x072000)
                curl_easy_setopt(handle, CURLOPT_NOPROGRESS,        0);
            }

        public:
            virtual bool   write(const void* ptr, size_t bytes) = 0;
            virtual size_t read(void* ptr, size_t bytes) = 0;
            virtual void   header(const void* ptr, size_t bytes) = 0;
            virtual bool   seek(int64_t offset, int origin) = 0;
            virtual bool   progress(size_t downCur, size_t downTotal, size_t downSpeed, size_t upCur, size_t upTotal, size_t upSpeed) = 0;
            virtual void   debug(int type, std::string const& msg) = 0;
            virtual void   finish(CURLcode code, int status) = 0;
            virtual void   cancel() = 0;

        public:
            void add_header(std::string const& key, std::string const& value) {
                std::string combined = key + ": " + value;
                headers = curl_slist_append(headers, combined.c_str());
                curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
            }

            void add_form_data(std::string const& key, http::buffer const& value, std::string const& type) {
                if(type.empty()) {
                    curl_formadd(
                        &post_data, &post_data_last,
                        CURLFORM_NAMELENGTH,        key.size(),
                        CURLFORM_PTRNAME,           key.data(),
                        CURLFORM_CONTENTSLENGTH,    value.size(),
                        CURLFORM_PTRCONTENTS,       value.data(),
                        CURLFORM_END
                    );
                } else {
                    curl_formadd(
                        &post_data, &post_data_last,
                        CURLFORM_NAMELENGTH,        key.size(),
                        CURLFORM_PTRNAME,           key.data(),
                        CURLFORM_CONTENTSLENGTH,    value.size(),
                        CURLFORM_PTRCONTENTS,       value.data(),
                        CURLFORM_CONTENTTYPE,       type.c_str(),
                        CURLFORM_END
                    );
                }
            }

        private:
            static size_t write_stub(void* ptr, size_t size, size_t nmemb, void* userdata) {
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto bytes = size * nmemb;
                return (wrap->write(ptr, bytes) ? bytes : 0);
            }

            static  size_t read_stub(void* ptr, size_t size, size_t nmemb, void* userdata) {
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto bytes = size * nmemb;
                return wrap->read(ptr, bytes);
            }

            static size_t header_stub(void* ptr, size_t size, size_t nmemb, void* userdata) {
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto bytes = size * nmemb;
                wrap->header(ptr, bytes);
                return bytes;
            }

            static int seek_stub(void* userdata, curl_off_t offset, int origin) {
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto off = static_cast<int64_t>(offset);
                return (wrap->seek(off, origin) ? CURL_SEEKFUNC_OK : CURL_SEEKFUNC_FAIL);
            }

            static int debug_stub(CURL* handle, curl_infotype type, const char* msg, size_t bytes, void* userdata) {
                (void)handle;
                auto wrap = static_cast<curl_easy_wrap*>(userdata); assert(wrap);
                auto str = std::string(msg, bytes); // add a terminating zero to the message
                wrap->debug(static_cast<int>(type), str);
                return 0;
            }

#if (LIBCURL_VERSION_NUM >= 0x072000)
            static int progress_stub(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
#else // (LIBCURL_VERSION_NUM >= 0x072000)
            static int progress_stub(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
#endif // (LIBCURL_VERSION_NUM >= 0x072000)
                auto wrap = static_cast<curl_easy_wrap*>(clientp); assert(wrap);

                double speedDown = 0.0, speedUp = 0.0;
                curl_easy_getinfo(wrap->handle, CURLINFO_SPEED_DOWNLOAD, &speedDown);
                curl_easy_getinfo(wrap->handle, CURLINFO_SPEED_UPLOAD,   &speedUp);
                
                auto res = wrap->progress(
                    static_cast<size_t>(dlnow), static_cast<size_t>(dltotal), static_cast<size_t>(speedDown),
                    static_cast<size_t>(ulnow), static_cast<size_t>(ultotal), static_cast<size_t>(speedUp)
                );
                return (res ? 0 : 1);
            }

        public:
            CURL* const     handle;
            curl_slist*     headers;
            curl_httppost*  post_data;
            curl_httppost*  post_data_last;
            char            error_buffer[CURL_ERROR_SIZE];

        private:
            curl_easy_wrap(curl_easy_wrap const&); // = delete;
            curl_easy_wrap& operator=(curl_easy_wrap const&); // = delete;
        };

    } // namespace impl
} // namespace http
