#include "../../include/webserv.hpp"

void initGET(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &get) {
	get["/"] = handleHome;
	get["/assets/img/42Wolfsburg_Logo_ver_pos_black.pdf.jpg"] = handleIndexImgLogo;
	get["/assets/img/atoepper.png"] = handleIndexImgAtoepper;
	get["/assets/img/jkarras.png"] = handleIndexImgJkarras;
	get["/assets/img/rmatthes.png"] = handleIndexImgRmathes;
	get["/assets/css/index_style.css"] = handleIndexSstyle;
	get["/assets/js/dashboard.js"] = handleDashboardJs;
	get["/assets/css/dashboard_style.css"] = handleDashboardStyle;
	get["/assets/js/index.js"] = handleIndexJs;
	get["/assets/js/remote_storage.js"] = handleRemoteStorageJs;
	get["/dashboard"] = handleDashboard;
}

void initPOST(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &post) {
	post["/auth/login"] = handleLogin;
	post["/createAccount"] = handleCreateAccount;
	post["/uploadFile"] = uploadFile;
}

void initDEL(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &del) {
	del.clear();
}

void initRoutes(ServerContext &serverContext) {
	initGET(serverContext.get);
	initPOST(serverContext.post);
	initDEL(serverContext.del);
}
