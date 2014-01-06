var http = require("http");
var url = require("url");

function start(route, handle) {
	function onRequest(request, response) {
		var pathname = url.parse(request.url).pathname;
//		console.log("Request for " + pathname + " received.");

		route(handle, pathname, request, response);
	}

	http.createServer(onRequest).listen(8888);
	console.log("Local node.js server has been started on port 8888.");
}

exports.start = start;
