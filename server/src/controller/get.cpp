#include "../../include/webserv.hpp"

void handle400(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "Http bad request";
	res.body = getFileContent("public/error/400.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 400;
}

void handle403(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "Http frobidden";
	res.body = getFileContent("public/error/403.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 403;
}

void handle404(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "Http not found";
	res.body = getFileContent("public/error/404.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 404;
}

void handle405(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "Http method not allowed";
	res.body = getFileContent("public/error/405.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 405;
}

void handle500(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "Http internl server error";
	res.body = getFileContent("public/error/500.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 500;
}

void handleHome(HttpRequest &req, HttpResponse &res) {
	res.version = req.version;
	res.state = SENDING_HEADERS;
	res.statusMessage = "OK";
	res.body = getFileContent("public/index.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}
