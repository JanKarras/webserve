#include "../../include/webserv.hpp"

void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage) {
	res.body = getFileContent(filePath);
	res.statusMessage = defaultMessage;
	res.headers["Content-Type"] = contentType;
	res.headers["Content-Length"] = toStringInt(res.body.size());
	if (statusCode == 405) {
		res.headers["Allow"] = "GET";
	}
	res.statusCode = statusCode;
}

void handle400(HttpResponse &res) {
	handleFileResponse(res, "public/error/400.html", "text/html", 400, "Http bad request");
}

void handle401(HttpResponse &res) {
	handleFileResponse(res, "public/error/401.html", "text/html", 401, "Http unauthorized");
}

void handle403(HttpResponse &res) {
	handleFileResponse(res, "public/error/403.html", "text/html", 403, "Http forbidden");
}

void handle404(HttpResponse &res) {
	handleFileResponse(res, "public/error/404.html", "text/html", 404, "Http not found");
}

void handle405(HttpResponse &res) {
	handleFileResponse(res, "public/error/405.html", "text/html", 405, "Method not allowed");
}

void handle500(HttpResponse &res) {
	handleFileResponse(res, "public/error/500.html", "text/html", 500, "Http internal server error");
}

void handle501(HttpResponse &res) {
	handleFileResponse(res, "public/error/501.html", "text/html", 501, "501 Not Implemented");
}

void handle504(HttpResponse &res) {
	handleFileResponse(res, "public/error/501.html", "text/html", 504, "Timeout");
}

void handle502(HttpResponse &res) {
	handleFileResponse(res, "public/error/501.html", "text/html", 502, "Bad Gateway");
}

void handle413(HttpResponse &res) {
	handleFileResponse(res, "public/error/413.html", "text/html", 413, "413 Payload Too Large");
}

void handleHome(HttpResponse &res) {
	handleFileResponse(res, "public/index.html", "text/html", 200, "OK");
}

void handleDashboard(HttpResponse &res) {
	handleFileResponse(res, "public/dashboard.html", "text/html", 200, "OK");
}

void handleIndexSstyle(HttpResponse &res) {
	handleFileResponse(res, "public/assets/css/index_style.css", "text/css", 200, "OK");
}

void handleIndexJs(HttpResponse &res) {
	handleFileResponse(res, "public/assets/js/index.js", "application/javascript", 200, "OK");
}

void handleRemoteStorageJs(HttpResponse &res) {
	handleFileResponse(res, "public/assets/js/remote_storage.js", "application/javascript", 200, "OK");
}

void handleIndexImgJkarras(HttpResponse &res) {
	handleFileResponse(res, "public/assets/img/jkarras.png", "image/png", 200, "OK");
}

void handleIndexImgAtoepper(HttpResponse &res) {
	handleFileResponse(res, "public/assets/img/atoepper.png", "image/png", 200, "OK");
}

void handleIndexImgRmathes(HttpResponse &res) {
	handleFileResponse(res, "public/assets/img/rmatthes.png", "image/png", 200, "OK");
}

void handleIndexImgLogo(HttpResponse &res) {
	handleFileResponse(res, "public/assets/img/42Wolfsburg_Logo_ver_pos_black.pdf.jpg", "image/jpeg", 200, "OK");
}

void handleDashboardStyle(HttpResponse &res) {
	handleFileResponse(res, "public/assets/css/dashboard_style.css", "text/css", 200, "OK");
}

void handleDashboardJs(HttpResponse &res) {
	handleFileResponse(res, "public/assets/js/dashboard.js", "application/javascript", 200, "OK");
}
