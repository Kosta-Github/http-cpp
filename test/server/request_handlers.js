function register_handlers(handle) {

    handle["/start"] = function(request, response) {
        response.writeHead(200, { "Content-Type": "text/plain" });
        response.write("Server started");
        response.end();
    }

    handle["/stop"] = function(request, response) {
        response.writeHead(200, { "Content-Type": "text/plain" });
        response.write("Server stopped");
        response.end();
        
        process.exit();
    }
    
    handle["/HTTP_200_OK"] = function (request, response) {
        response.writeHead(200, { "Content-Type": "text/plain" });
        response.write("URL found");
        response.end();
    }

    handle["/HTTP_404_NOT_FOUND"] = function (request, response) {
        response.writeHead(404, { "Content-Type": "text/plain" });
        response.write("URL not found");
        response.end();
    }

    handle["/stream"] = function (request, response) {
        var i;
        response.writeHead(200, { "Content-Type": "text/plain" });
        for (i = 0; i < 200; ++i) {
            response.write("streaming #" + i + "\n");
        }
        response.end();
    }

    handle["/delay"] = function (request, response) {
        setTimeout(function () {
            response.writeHead(200, { "Content-Type": "text/plain" });
            response.write("response delayed");
            response.end();
        }, 3000); // wait 3 second before responding
    }

    handle["/echo_headers"] = function (request, response) {
        var headers = request.headers;
        headers["Content-Type"] = "text/plain";
        response.writeHead(200, headers);
        response.write("headers received");
        response.end();
    }

    handle["/echo_request"] = function (request, response) {
        response.writeHead(200, { "Content-Type": "text/plain" });

        var data = "";
        request.setEncoding('utf8');
        request.on('data', function (chunk) { data += chunk; });
        request.on('end', function () {
            response.write(request.method + " received: " + data);
            response.end();
        });
    }

}

exports.register_handlers = register_handlers;
