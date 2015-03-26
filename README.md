http-cpp [![Build Status Travis](https://travis-ci.org/Kosta-Github/http-cpp.png)](https://travis-ci.org/Kosta-Github/http-cpp) [![Coverity Scan](https://scan.coverity.com/projects/1344/badge.svg)](https://scan.coverity.com/projects/1344) [![Build status AppVeyor](https://ci.appveyor.com/api/projects/status/6is9qo2njtro850d/branch/master?svg=true)](https://ci.appveyor.com/project/Kosta-Github/http-cpp/branch/master)
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

OAuth1 sample
=============
```
auto client = http::oauth1::client();
client.consumer_key    = "...";
client.consumer_secret = "...";
client.token_key       = "...";
client.token_secret    = "...";
auto reply = client.request("https://do.an.oauth1.request.com").data(); // async request
// do some more work in the meantime
const std::string& body = reply.get().body; // retrieve the reply body
```

external dependencies
=====================
- [curl](http://curl.haxx.se/): currently using version `7.41.0` for `Windows` builds
- [HMAC_SHA1](http://www.codeproject.com/KB/recipes/HMACSHA1class.aspx): `HMAC_SHA1` library for `OAuth1` support
- [base64](http://www.adp-gmbh.ch/cpp/common/base64.html): `Base64` library for `OAuth1` support
- [cmake](http://cmake.org): for the build system
- [cute](https://github.com/Kosta-Github/cute): only for unit tests
- [node.js](http://nodejs.org/): only for unit tests
