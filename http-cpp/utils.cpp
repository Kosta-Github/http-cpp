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

#include "./utils.hpp"

#include <cassert>

static const char HEX[] = "0123456789ABCDEF";

static inline bool is_hex(
    int const h
) {
    switch(h) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            return true;
        default:
            return false;
    }
}

static inline int hex2int(
    int const h
) {
    assert(is_hex(h));
    switch(h) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return (h - '0');
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            return ((h - 'A') + 10);
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            return ((h - 'a') + 10);
        default:
            return 0;
    }
}

namespace {
    enum encode_type {
        encode_all_type,
        encode_path_type,
        encode_key_type,
        encode_value_type
    };
}

static std::string encode(
    std::string const& str,
    encode_type const type
) {
    std::string escaped;
    escaped.reserve(3 * str.size());

    for(unsigned char const c : str) {
        bool enc = true;
        switch(c) {
            // these don't need to be encoded
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':

            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':

            case '~': case '-': case '_': case '.':
                        enc = false; break;

            case '=':   enc = (type == encode_all_type) || (type == encode_key_type); break; // encode for keys
            case '&':   enc = (type != encode_path_type); break; // encode for keys and values
            case '+':   enc = (type != encode_path_type); break; // encode for keys and values
            case '/':   enc = (type != encode_path_type); break; // encode for keys and values

            // by default encode all other chars not handle above
            default:    enc = true; break;
        }

        if(enc) {
            int const high = (c & 0xF0) >> 4;
            int const low  = (c & 0x0F);

            escaped += '%';
            escaped += HEX[high];
            escaped += HEX[low];
        } else {
            escaped += c;
        }
    }

    return escaped;
}

static const std::string HTTP_PREFIX  = "http://";
static const std::string HTTPS_PREFIX = "https://";

std::string http::encode_path(std::string const& path) {
    if(path.compare(0, HTTP_PREFIX.size(),  HTTP_PREFIX)  == 0) { return (HTTP_PREFIX  + encode(path.substr(HTTP_PREFIX.size()),  encode_path_type)); }
    if(path.compare(0, HTTPS_PREFIX.size(), HTTPS_PREFIX) == 0) { return (HTTPS_PREFIX + encode(path.substr(HTTPS_PREFIX.size()), encode_path_type)); }
    return encode(path, encode_path_type);
}

std::string http::encode_key(std::string const& key) {
    return encode(key, encode_key_type);
}

std::string http::encode_value(std::string const& value) {
    return encode(value, encode_value_type);
}

std::string http::encode_all(std::string const& str) {
    return encode(str, encode_all_type);
}

std::string http::decode(
    std::string const& str
) {
    std::string unescaped;
    unescaped.reserve(str.size());

    for(size_t i = 0, iEnd = str.size(); i < iEnd; ++i) {
        const auto c = str[i];
        if((c == '%') && (i + 2 < iEnd) && is_hex(str[i+1]) && is_hex(str[i+2])) {
            const int high = hex2int(str[i+1]);
            const int low  = hex2int(str[i+2]);
            unescaped += static_cast<unsigned char>(high << 4 | low);
            i += 2;
        } else {
            unescaped += c;
        }
    }

    return unescaped;
}
