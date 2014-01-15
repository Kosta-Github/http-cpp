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

namespace http {

    // see: http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    enum status {
        HTTP_000_UNKNOWN                                = 000,

        HTTP_100_CONTINUE                               = 100,
        HTTP_101_SWITCHING_PROTOCOLS                    = 101,
        HTTP_102_PROCESSING                             = 102,

        HTTP_200_OK                                     = 200,
        HTTP_201_CREATED                                = 201,
        HTTP_202_ACCEPTED                               = 202,
        HTTP_203_NON_AUTHORITATIVE_INFORMATION          = 203,
        HTTP_204_NO_CONTENT                             = 204,
        HTTP_205_RESET_CONTENT                          = 205,
        HTTP_206_PARTIAL_CONTENT                        = 206,
        HTTP_207_MULTI_STATUS                           = 207,
        HTTP_208_ALREADY_REOPENED                       = 208,
        HTTP_226_IM_USED                                = 226,

        HTTP_300_MULTIPLE_CHOICES                       = 300,
        HTTP_301_MOVED_PERMANENTLY                      = 301,
        HTTP_302_FOUND                                  = 302,
        HTTP_303_SEE_OTHER                              = 303,
        HTTP_304_NOT_MODIFIED                           = 304,
        HTTP_305_USE_PROXY                              = 305,
        HTTP_306_SWITCH_PROXY                           = 306,
        HTTP_307_TEMPORARY_REDIRECT                     = 307,
        HTTP_308_PERMANENT_REDIRECT                     = 308,

        HTTP_400_BAD_REQUEST                            = 400,
        HTTP_401_UNAUTHORIZED                           = 401,
        HTTP_402_PAYMENT_REQUIRED                       = 402,
        HTTP_403_FORBIDDEN                              = 403,
        HTTP_404_NOT_FOUND                              = 404,
        HTTP_405_METHOD_NOT_ALLOWED                     = 405,
        HTTP_406_NOT_ACCECPTABLE                        = 406,
        HTTP_407_PROXY_AUTHENTICATION_REQUIRED          = 407,
        HTTP_408_REQUEST_TIMEOUT                        = 408,
        HTTP_409_CONFLICT                               = 409,
        HTTP_410_GONE                                   = 410,
        HTTP_411_LENGTH_REQUIRED                        = 411,
        HTTP_412_PRECONDITION_FAILED                    = 412,
        HTTP_413_REQUEST_ENTITY_TOO_LARGE               = 413,
        HTTP_414_REQUEST_URI_TOO_LONG                   = 414,
        HTTP_415_UNSUPPORTED_MEDIA_TYPE                 = 415,
        HTTP_416_REQUEST_RANGE_NOT_SATISFIABLE          = 416,
        HTTP_417_EXPECTATION_FAILED                     = 417,
        HTTP_418_I_AM_A_TEAPOT                          = 418,
        HTTP_419_AUTHENTICATION_TIMEOUT                 = 419,
        HTTP_420_METHOD_FAILURE                         = 420,
        HTTP_420_ENHANCE_YOUR_CALM                      = 420,
        HTTP_422_UNPROCESSABLE_ENTITY                   = 422,
        HTTP_423_LOCKED                                 = 423,
        HTTP_424_FAILED_DEPENDENCY                      = 424,
        HTTP_425_UNORDERED_COLLECTION                   = 425,
        HTTP_426_UPGRADE_REQUIRED                       = 426,
        HTTP_428_PRECONDITION_REQUIRED                  = 428,
        HTTP_429_TOO_MANY_REQUESTS                      = 429,
        HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE        = 431,
        HTTP_440_LOGIN_TIMEOUT                          = 440,
        HTTP_444_NO_RESPONSE                            = 444,
        HTTP_449_RETRY_WITH                             = 449,
        HTTP_450_BLOCKED_BY_WINDOWS_PARENTAL_CONTROLS   = 450,
        HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS          = 451,
        HTTP_451_REDIRECTED                             = 451,
        HTTP_494_REQUEST_HEADER_TOO_LARGE               = 494,
        HTTP_495_CERT_ERROR                             = 495,
        HTTP_496_NO_CERT                                = 496,
        HTTP_497_HTTP_TO_HTTPS                          = 497,
        HTTP_499_CLIENT_CLOSED_REQUEST                  = 499,

        HTTP_500_INTERNAL_SERVER_ERROR                  = 500,
        HTTP_501_NOT_IMPLEMENTED                        = 501,
        HTTP_502_BAD_GATEWAY                            = 502,
        HTTP_503_SERVICE_UNAVAILABLE                    = 503,
        HTTP_504_GATEWAY_TIMEOUT                        = 504,
        HTTP_505_HTTP_VERSION_NOT_SUPPORTED             = 505,
        HTTP_506_VARIANT_ALSO_NEGOTIATES                = 506,
        HTTP_507_INSUFFICIENT_STORAGE                   = 507,
        HTTP_508_LOOP_DETECTED                          = 508,
        HTTP_509_BANDWIDTH_LIMIT_EXCEEDED               = 509,
        HTTP_510_NOT_EXTENDED                           = 510,
        HTTP_511_NETWORK_AUTHENTICATION_REQUIRED        = 511,
        HTTP_520_ORIGIN_ERROR                           = 520,
        HTTP_522_CONNECTION_TIMED_OUT                   = 522,
        HTTP_523_PROXY_DECLINED_REQUEST                 = 523,
        HTTP_524_TIMEOUT_OCCURRED                       = 524,
        HTTP_598_NETWORK_READ_TIMEOUT_ERROR             = 598,
        HTTP_599_NETWORK_CONNECT_TIMEOUT_ERROR          = 599
    };

    HTTP_API bool status_is_known(http::status s);

    HTTP_API const char* status_to_string(http::status s);
    
} // namespace http
