function register_handlers(handle) {

	handle["/start"] = function(request, response) {
		response.writeHead(200, {"Content-Type": "text/html"});
		response.write("Server started");
		response.end();
	}

	handle["/stop"] = function(request, response) {
		response.writeHead(200, {"Content-Type": "text/plain"});
		response.write("Server stopped");
		response.end();
		
		process.exit();
	}
	
	handle["/HTTP_200_OK"] = function (request, response) {
	    response.writeHead(200, { "Content-Type": "text/html" });
	    response.write("URL found");
	    response.end();
	}

	handle["/HTTP_404_NOT_FOUND"] = function (request, response) {
	    response.writeHead(404, { "Content-Type": "text/html" });
	    response.write("URL not found");
	    response.end();
	}

	handle["/stream"] = function (request, response) {
	    var i;
	    response.writeHead(200, { "Content-Type": "text/html" });
	    for (i = 0; i < 200; ++i) {
	        response.write("streaming #" + i + "\n");
	    }
	    response.end();
	}

	handle["/delay"] = function (request, response) {
	    setTimeout(function () {
	        response.writeHead(200, { "Content-Type": "text/html" });
	        response.write("response delayed");
	        response.end();
	    }, 1000); // wait 1 second before responding
	}

}

exports.register_handlers = register_handlers;
