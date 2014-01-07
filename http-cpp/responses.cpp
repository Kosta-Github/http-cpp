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

#include "responses.hpp"

http::response http::responses::add(
    http::response response
) {
    m_responses.emplace_back(response);
    return response;
}

http::progress http::responses::progress_all() const {
    http::progress result;

    bool downTotalValid = true;
    bool upTotalValid   = true;

    for(auto&& r : m_responses) {
        auto p = r.progress();

        result.downloadCurrentBytes += p.downloadCurrentBytes;
        result.downloadTotalBytes   += p.downloadTotalBytes;
        result.downloadSpeed        += p.downloadSpeed;

        result.uploadCurrentBytes   += p.uploadCurrentBytes;
        result.uploadTotalBytes     += p.uploadTotalBytes;
        result.uploadSpeed          += p.uploadSpeed;

        downTotalValid  = (downTotalValid   && (p.downloadTotalBytes    > 0));
        upTotalValid    = (upTotalValid     && (p.uploadTotalBytes      > 0));
    }

    if(!downTotalValid) { result.downloadTotalBytes = 0; }
    if(!upTotalValid)   { result.uploadTotalBytes   = 0; }

    return result;
}

void http::responses::cancel_all() {
    for(auto&& r : m_responses) {
        r.cancel();
    }
}

void http::responses::wait_all() {
    for(auto&& r : m_responses) {
        r.data().wait();
    }
}
