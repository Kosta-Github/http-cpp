#pragma once

#include "http-cpp.hpp"

#include <cstdlib>

namespace http {

    struct progress_info {
        progress_info(
            size_t dlCur = 0, size_t dlTot = 0, size_t dlSpd = 0,
            size_t ulCur = 0, size_t ulTot = 0, size_t upSpd = 0
        ) :
            downloadCurrentBytes(dlCur), downloadTotalBytes(dlTot), downloadSpeed(dlSpd),
            uploadCurrentBytes(ulCur), uploadTotalBytes(ulTot), uploadSpeed(upSpd)
        { }

        size_t downloadCurrentBytes;
        size_t downloadTotalBytes;
        size_t downloadSpeed; // bytes/sec
        size_t uploadCurrentBytes;
        size_t uploadTotalBytes;
        size_t uploadSpeed; // bytes/sec
    };

} // namespace http
