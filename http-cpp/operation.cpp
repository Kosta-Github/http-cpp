#include "operation.hpp"

bool http::is_known(http::operation op) {
    switch(op) {
#define HTTP_OPERATION_CASE(X) case X: return true
        HTTP_OPERATION_CASE(HTTP_GET);
        HTTP_OPERATION_CASE(HTTP_HEAD);
        HTTP_OPERATION_CASE(HTTP_POST);
        HTTP_OPERATION_CASE(HTTP_PUT);
        HTTP_OPERATION_CASE(HTTP_DELETE);
#undef  HTTP_OPERATION_CASE
        default: return false;
    }
}

const char* http::to_string(http::operation op) {
    switch(op) {
#define HTTP_OPERATION_CASE(X) case X: return #X
        HTTP_OPERATION_CASE(HTTP_GET);
        HTTP_OPERATION_CASE(HTTP_HEAD);
        HTTP_OPERATION_CASE(HTTP_POST);
        HTTP_OPERATION_CASE(HTTP_PUT);
        HTTP_OPERATION_CASE(HTTP_DELETE);
#undef  HTTP_OPERATION_CASE
        default: return "HTTP_UNKNOWN";
    }
}
