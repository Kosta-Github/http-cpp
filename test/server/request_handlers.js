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
        }, 1000); // wait 1 second before responding
    }

    handle["/get_request"] = function (request, response) {
        var status = ((request.method === "GET") ? 200 : 404);
        response.writeHead(status, { "Content-Type": "text/plain" });
        response.write(request.method + " received");
        response.end();
    }

    handle["/head_request"] = function (request, response) {
        var status = ((request.method === "HEAD") ? 200 : 404);
        response.writeHead(status, { "Content-Type": "text/plain" });
        response.write(request.method + " received");
        response.end();
    }

    handle["/put_request"] = function (request, response) {
        var status = ((request.method === "PUT") ? 200 : 404);
        response.writeHead(status, { "Content-Type": "text/plain" });

        var data = "";
        request.setEncoding('utf8');
        request.on('data', function (chunk) { data += chunk; });
        request.on('end', function () {
            response.write(request.method + " received: " + data);
            response.end();
        });
    }

    handle["/delete_request"] = function (request, response) {
        var status = ((request.method === "DELETE") ? 200 : 404);
        response.writeHead(status, { "Content-Type": "text/plain" });
        response.write(request.method + " received");
        response.end();
    }

}

exports.register_handlers = register_handlers;
