#pragma once

#if defined(WIN32)
#  if defined(http_cpp_EXPORTS)
#    define HTTP_API __declspec(dllexport)
#  else // defined(http_cpp_EXPORTS)
#    define HTTP_API __declspec(dllimport)
#    pragma comment(lib, "http-cpp.lib")
#  endif // defined(http_cpp_EXPORTS)
#else // defined(WIN32)
#  define HTTP_API
#endif // defined(WIN32)

#if defined(_MSC_VER) && (_MSC_VER <= 1800)
#  define HTTP_CPP_NOEXCEPT throw()
#  define HTTP_CPP_NEED_EXPLICIT_MOVE
#else // defined(_MSC_VER) && (_MSC_VER <= 1800)
#  define HTTP_CPP_NOEXCEPT noexcept
#endif // defined(_MSC_VER) && (_MSC_VER <= 1800)

namespace http { }

/*
1) fire-and-forget request
2) manage body content for a request? lifetime/ownership?
3) progress
4) cancel?
5) response support streaming?
6) status/header available before body
*/
