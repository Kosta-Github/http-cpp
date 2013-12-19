var server = require("./server");
var router = require("./router");
var requestHandlers = require("./request_handlers");

var handle = { };
requestHandlers.register_handlers(handle);

server.start(router.route, handle);
