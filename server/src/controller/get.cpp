#include "../../include/webserv.hpp"

void handle400(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "Http bad request";
	res.body = getFileContent("public/error/400.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 400;
}

void handle401(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "Http unauthorized";
	res.body = getFileContent("public/error/401.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 401;
}

void handle403(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
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
	res.statusCode = req.exitStatus;
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
	res.statusCode = req.exitStatus;
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
	res.statusCode = req.exitStatus;
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
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/index.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleDashboard(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/dashboard.html");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexSstyle(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/css/index_style.css");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/css";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexJs(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/js/index.js");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/js";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleRemoteStorageJs(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/js/remote_storage.js");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/js";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexImgJkarras(HttpRequest &req, HttpResponse &res) {

	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/img/jkarras.png");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "image/png";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexImgAtoepper(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/img/atoepper.png");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "image/png";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexImgRmathes(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/img/rmatthes.png");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "image/png";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleIndexImgLogo(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/img/42Wolfsburg_Logo_ver_pos_black.pdf.jpg");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "image/jpg";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleDashboardStyle(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/css/dashboard_style.css");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/css";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}

void handleDashboardJs(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	res.statusMessage = "OK";
	res.body = getFileContent("public/assets/js/dashboard.js");
	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/js";
	res.headers["Content-Length"] = toStringInt(res.body.size());
	res.statusCode = 200;
}
