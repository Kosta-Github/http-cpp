http-cpp [![Build Status](https://travis-ci.org/Kosta-Github/http-cpp.png)](https://travis-ci.org/Kosta-Github/http-cpp)
========
Provide a very simple and thread-safe interface to do HTTP requests in C++11.

sample
======
```
auto reply = http::client().request("http://www.google.com").data().get();
const std::string& body = reply.body;
```

external dependencies
=====================
- [curl](http://curl.haxx.se/)
- [cmake](http://cmake.org) (for the build system)
- [catch](https://github.com/philsquared/Catch) (only for unit tests)
- [node.js](http://nodejs.org/) (only for unit tests)
