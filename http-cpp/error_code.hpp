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

#include "http-cpp.hpp"

#include <string>

namespace http {

    // an error_code <= 0 indicates a "no error situation"
    // a value > 0 should be treated as "an error occurred"
    enum error_code {
        HTTP_ERROR_OK = 0,
        HTTP_ERROR_REPORT_PROGRESS = -1, // no error
        HTTP_ERROR_REQUEST_CANCELED = 3002,

        // from libcurl: see CURLcode
        HTTP_ERROR_UNSUPPORTED_PROTOCOL     = 1,
        HTTP_ERROR_FAILED_INIT              = 2,
        HTTP_ERROR_URL_MALFORMAT            = 3,
        HTTP_ERROR_NOT_BUILT_IN             = 4,
        HTTP_ERROR_COULDNT_RESOLVE_PROXY    = 5,
        HTTP_ERROR_COULDNT_RESOLVE_HOST     = 6,
        HTTP_ERROR_COULDNT_CONNECT          = 7,
        HTTP_ERROR_FTP_WEIRD_SERVER_REPLY   = 8,
        HTTP_ERROR_REMOTE_ACCESS_DENIED     = 9,
        HTTP_ERROR_FTP_ACCEPT_FAILED        = 10,
        HTTP_ERROR_FTP_WEIRD_PASS_REPLY     = 11,
        HTTP_ERROR_FTP_ACCEPT_TIMEOUT       = 12,
        HTTP_ERROR_FTP_WEIRD_PASV_REPLY     = 13,
        HTTP_ERROR_FTP_WEIRD_227_FORMAT     = 14,
        HTTP_ERROR_FTP_CANT_GET_HOST        = 15,
        HTTP_ERROR_OBSOLETE16               = 16,
        HTTP_ERROR_FTP_COULDNT_SET_TYPE     = 17,
        HTTP_ERROR_PARTIAL_FILE             = 18,
        HTTP_ERROR_FTP_COULDNT_RETR_FILE    = 19,
        HTTP_ERROR_OBSOLETE20               = 20,
        HTTP_ERROR_QUOTE_ERROR              = 21,
        HTTP_ERROR_HTTP_RETURNED_ERROR      = 22,
        HTTP_ERROR_WRITE_ERROR              = 23,
        HTTP_ERROR_OBSOLETE24               = 24,
        HTTP_ERROR_UPLOAD_FAILED            = 25,
        HTTP_ERROR_READ_ERROR               = 26,
        HTTP_ERROR_OUT_OF_MEMORY            = 27,
        HTTP_ERROR_OPERATION_TIMEDOUT       = 28,
        HTTP_ERROR_OBSOLETE29               = 29,
        HTTP_ERROR_FTP_PORT_FAILED          = 30,
        HTTP_ERROR_FTP_COULDNT_USE_REST     = 31,
        HTTP_ERROR_OBSOLETE32               = 32,
        HTTP_ERROR_RANGE_ERROR              = 33,
        HTTP_ERROR_HTTP_POST_ERROR          = 34,
        HTTP_ERROR_SSL_CONNECT_ERROR        = 35,
        HTTP_ERROR_BAD_DOWNLOAD_RESUME      = 36,
        HTTP_ERROR_FILE_COULDNT_READ_FILE   = 37,
        HTTP_ERROR_LDAP_CANNOT_BIND         = 38,
        HTTP_ERROR_LDAP_SEARCH_FAILED       = 39,
        HTTP_ERROR_OBSOLETE40               = 40,
        HTTP_ERROR_FUNCTION_NOT_FOUND       = 41,
        HTTP_ERROR_ABORTED_BY_CALLBACK      = 42,
        HTTP_ERROR_BAD_FUNCTION_ARGUMENT    = 43,
        HTTP_ERROR_OBSOLETE44               = 44,
        HTTP_ERROR_INTERFACE_FAILED         = 45,
        HTTP_ERROR_OBSOLETE46               = 46,
        HTTP_ERROR_TOO_MANY_REDIRECTS       = 47,
        HTTP_ERROR_UNKNOWN_OPTION           = 48,
        HTTP_ERROR_TELNET_OPTION_SYNTAX     = 49,
        HTTP_ERROR_OBSOLETE50               = 50,
        HTTP_ERROR_PEER_FAILED_VERIFICATION = 51,
        HTTP_ERROR_GOT_NOTHING              = 52,
        HTTP_ERROR_SSL_ENGINE_NOTFOUND      = 53,
        HTTP_ERROR_SSL_ENGINE_SETFAILED     = 54,
        HTTP_ERROR_SEND_ERROR               = 55,
        HTTP_ERROR_RECV_ERROR               = 56,
        HTTP_ERROR_OBSOLETE57               = 57,
        HTTP_ERROR_SSL_CERTPROBLEM          = 58,
        HTTP_ERROR_SSL_CIPHER               = 59,
        HTTP_ERROR_SSL_CACERT               = 60,
        HTTP_ERROR_BAD_CONTENT_ENCODING     = 61,
        HTTP_ERROR_LDAP_INVALID_URL         = 62,
        HTTP_ERROR_FILESIZE_EXCEEDED        = 63,
        HTTP_ERROR_USE_SSL_FAILED           = 64,
        HTTP_ERROR_SEND_FAIL_REWIND         = 65,
        HTTP_ERROR_SSL_ENGINE_INITFAILED    = 66,
        HTTP_ERROR_LOGIN_DENIED             = 67,
        HTTP_ERROR_TFTP_NOTFOUND            = 68,
        HTTP_ERROR_TFTP_PERM                = 69,
        HTTP_ERROR_REMOTE_DISK_FULL         = 70,
        HTTP_ERROR_TFTP_ILLEGAL             = 71,
        HTTP_ERROR_TFTP_UNKNOWNID           = 72,
        HTTP_ERROR_REMOTE_FILE_EXISTS       = 73,
        HTTP_ERROR_TFTP_NOSUCHUSER          = 74,
        HTTP_ERROR_CONV_FAILED              = 75,
        HTTP_ERROR_CONV_REQD                = 76,
        HTTP_ERROR_SSL_CACERT_BADFILE       = 77,
        HTTP_ERROR_REMOTE_FILE_NOT_FOUND    = 78,
        HTTP_ERROR_SSL_SHUTDOWN_FAILED      = 80,
        HTTP_ERROR_AGAIN                    = 81,
        HTTP_ERROR_SSL_CRL_BADFILE          = 82,
        HTTP_ERROR_SSL_ISSUER_ERROR         = 83,
        HTTP_ERROR_FTP_PRET_FAILED          = 84,
        HTTP_ERROR_RTSP_CSEQ_ERROR          = 85,
        HTTP_ERROR_RTSP_SESSION_ERROR       = 86,
        HTTP_ERROR_FTP_BAD_FILE_LIST        = 87,
        HTTP_ERROR_CHUNK_FAILED             = 88,
        HTTP_ERROR_NO_CONNECTION_AVAILABLE  = 89,

        // from libcurl: see CURLMcode
        HTTP_ERROR_BAD_HANDLE               = 1001,
        HTTP_ERROR_BAD_EASY_HANDLE          = 1002,
        HTTP_ERROR_OUT_OF_MEMORY2           = 1003,
        HTTP_ERROR_INTERNAL_ERROR           = 1004,
        HTTP_ERROR_BAD_SOCKET               = 1005,
        HTTP_ERROR_UNKNOWN_OPTION2          = 1006,

        // from libcurl: see CURLSHcode
        HTTP_ERROR_BAD_OPTION               = 2001,
        HTTP_ERROR_IN_USE                   = 2002,
        HTTP_ERROR_INVALID                  = 2003,
        HTTP_ERROR_NOMEM                    = 2004,
        HTTP_ERROR_NOT_BUILT_IN2            = 2005
    };

    /// checks if the given `error_code` is an error
    HTTP_API bool is_error(http::error_code code);

    /// checks if the given `error_code` is known/valid
    HTTP_API bool is_valid(http::error_code code);

    /// returns an error string for the given `error_code`
    HTTP_API std::string to_string(http::error_code code);

} // namespace http
