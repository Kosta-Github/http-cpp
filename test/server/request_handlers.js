function register_handlers(handle) {

	handle["/start"] = function(request, response) {
		console.log("Request handler 'start' was called.");
		response.writeHead(200, {"Content-Type": "text/html"});
		response.write("Server started");
		response.end();
	}
	handle["/"] = handle["/start"];

	handle["/stop"] = function(request, response) {
		console.log("Request handler 'stop' was called.");
		response.writeHead(200, {"Content-Type": "text/plain"});
		response.write("Server stopped");
		response.end();
		
		process.exit();
	}
	
}

exports.register_handlers = register_handlers;
