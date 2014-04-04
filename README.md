http-cpp [![Build Status](https://travis-ci.org/Kosta-Github/http-cpp.png)](https://travis-ci.org/Kosta-Github/http-cpp)
========
Provide a very simple, thread-safe, and portable interface to do async HTTP requests in C++11.

sample
======
```
auto client = http::client();
auto reply = client.request("http://www.google.com").data(); // async request
// do some more work in the meantime
const std::string& body = reply.get().body; // retrieve the reply body
```

external dependencies
=====================
- [curl](http://curl.haxx.se/)
- [cmake](http://cmake.org): for the build system
- [cute](https://github.com/Kosta-Github/cute): only for unit tests
- [node.js](http://nodejs.org/): only for unit tests
