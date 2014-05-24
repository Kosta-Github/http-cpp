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

#include "./client.hpp"
#include "./requests.hpp"

http::request http::requests::request(
    http::client&   client,
    http::url       url,
    http::operation op
) {
    return add(client.request(std::move(url), std::move(op)));
}

http::request http::requests::add(
    http::request req
) {
    reqs.emplace_back(req);
    return req;
}

http::progress http::requests::progress_all() const {
    http::progress result;

    bool downTotalValid = true;
    bool upTotalValid   = true;

    for(auto&& r : reqs) {
        auto p = r.progress();

        result.downloadCurrentBytes += p.downloadCurrentBytes;
        result.downloadTotalBytes   += p.downloadTotalBytes;
        result.downloadSpeed        += p.downloadSpeed;

        result.uploadCurrentBytes   += p.uploadCurrentBytes;
        result.uploadTotalBytes     += p.uploadTotalBytes;
        result.uploadSpeed          += p.uploadSpeed;

        downTotalValid  = (downTotalValid   && ((p.downloadCurrentBytes == 0) || (p.downloadTotalBytes > 0)));
        upTotalValid    = (upTotalValid     && ((p.uploadCurrentBytes   == 0) || (p.uploadTotalBytes   > 0)));
    }

    if(!downTotalValid) { result.downloadTotalBytes = 0; }
    if(!upTotalValid)   { result.uploadTotalBytes   = 0; }

    return result;
}

void http::requests::cancel_all() {
    for(auto&& r : reqs) {
        r.cancel();
    }
}

void http::requests::wait_all() {
    for(auto&& r : reqs) {
        r.wait();
    }
}
