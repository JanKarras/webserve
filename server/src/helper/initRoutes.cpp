#include "../../include/webserv.hpp"

void initPAGES(std::map<std::string, void (*)(HttpResponse &)> &pages) {
	pages["/"] = handleHome;
	pages["/assets/img/42Wolfsburg_Logo_ver_pos_black.pdf.jpg"] = handleIndexImgLogo;
	pages["/assets/img/atoepper.png"] = handleIndexImgAtoepper;
	pages["/assets/img/jkarras.png"] = handleIndexImgJkarras;
	pages["/assets/img/rmatthes.png"] = handleIndexImgRmathes;
	pages["/assets/css/index_style.css"] = handleIndexSstyle;
	pages["/assets/js/dashboard.js"] = handleDashboardJs;
	pages["/assets/css/dashboard_style.css"] = handleDashboardStyle;
	pages["/assets/js/index.js"] = handleIndexJs;
	pages["/assets/js/remote_storage.js"] = handleRemoteStorageJs;
	pages["/dashboard"] = handleDashboard;
}

void initGET(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &get) {
	get["/getFile"] = handleGetFile;
	get["/getFileNames"] = getFileNames;
}

void initPOST(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &post) {
	post["/auth/login"] = handleLogin;
	post["/createAccount"] = handleCreateAccount;
	post["/uploadFile"] = uploadFile;
}

void initDEL(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &del) {
	del["/deleteFile"] = delteFile;
}

void initCGI(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &cgi) {
	cgi["/cgi/ls.sh"] = handleLs;
	cgi["/cgi/ls.php"] = handleLs;
	cgi["/cgi/ls.py"] = handleLs;
}

void initRoutes(ServerContext &serverContext) {
	initGET(serverContext.get);
	initPOST(serverContext.post);
	initDEL(serverContext.del);
	initCGI(serverContext.cgi);
	initPAGES(serverContext.pages);
}


