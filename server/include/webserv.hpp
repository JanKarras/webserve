#ifndef WEBSERV_HPP

#define WEBSERV_HPP

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <map>
#include <sstream>
#include <fstream>
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include "http_request.hpp"
#include "../classes/header/Logger.hpp"
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>

#define CR (u_char) 'r'
#define LF (u_char) 'n'

#define SUCCESS 0
#define FAILURE 1

#define LOOP 1

#define MAX_EVENTS 1024
#define MAX_CLIENTS 1024
#define PORT 8080
#define CHUNK_SIZE 1024
#define BUFFER_SIZE 1024
#define TIME_TO_KILL_CHILD 50
#define BIG_BODY_SIZE 10485760

#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_N_ALLOWED 405
#define HTTP_ENTITY_TOO_LARGE 413
#define HTTP_SERVER_ERROR 500


extern bool running;

enum ResponseState {
	NOT_STARTED,
	SENDING_HEADERS,
	SENDING_BODY,
	RESP_COMPLETE
};

struct HttpResponse {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;
	ResponseState state;
	size_t bodySent;
	long long startTime;

	HttpResponse() : version("HTTP/1.1"), statusCode(200), statusMessage("OK"), state(NOT_STARTED), bodySent(0), startTime(0) {}
};

struct file {
	std::string contentType;
	std::string path;
};

struct ServerContext {
	std::map<int, HttpRequest> requests;
	std::map<int, HttpResponse> responses;
	std::map<int, int> fds;
	std::map<int, pid_t> pids;
	std::vector<file> files;
};

struct errorPage {
	size_t errorCode;
	std::string path;
};

struct location {
	bool post;
	bool get;
	bool del;
	std::string name;
	std::string root;
	std::string index;
	std::vector<std::string> cgi_paths;
	std::vector<std::string> cgi_ext;
	std::string redirect;
	std::vector<file> files;
};

struct server{
	int port;
	size_t client_max_body_size;
	std::string server_name;
	std::string host;
	std::string root;
	std::string index;
	std::vector<errorPage> errorpages;
	std::vector<location> locations;
	ServerContext serverContex;
};

struct ConfigData {
	int epollFd;
	int serverFd;
	std::map<int, HttpRequest> requests;
	std::vector<server> servers;
	int port;
	struct sockaddr_in serverAddress;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];
};

void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage);

//CONFIC
bool parseConfic(std::string path, std::map<int, ConfigData> &data);
void printAll(std::map<int, ConfigData> &data);

//HTTPPARSER
void printHttpRequest(const HttpRequest& request);
void parseHttpRequest(ConfigData &config, int client_fd, std::string &data);

//SERVER
void startServer(std::map<int, ConfigData> &data);
bool initServer(ServerContext &ServerContext, struct sockaddr_in &serverAddress, struct epoll_event &event);
bool initServerConfic(ServerContext &ServerContext, struct sockaddr_in &serverAddress, struct epoll_event &event, ConfigData &conficData);
bool addEvent(ConfigData &configData);
bool handleEventReq(ConfigData &configData, int i);
bool handleEventRes(ConfigData &data, int i);
//SIG
void handle_sigint(int sig, siginfo_t *siginfo, void *context);
bool initSignal(void);
//HANDLE REQ
void handleRequest(int clientFd, ConfigData &configData);
void handleErrorRequest(int clientFd, ConfigData &configData, HttpRequest &req);
//helper
bool setsetExecutable(std::string &filePath);
std::string toStringInt(int number);
int toIntString(const std::string &str);
std::string toString(long long number);
std::string getDestPath(std::string email);
std::map<std::string, std::string> initMimeTypes( void );
long long getCurrentTime();
std::string getFileContent(std::string filePath);
void closeAll(ServerContext ServerContext);
int setNonBlocking(int fd);
void initRoutes(ServerContext &serverContext);
std::map<std::string, std::string> parseSimpleJSON(const std::string& body);
//POST ROUTES
void routeRequestPOST(HttpRequest &req, HttpResponse &res, ServerContext serverContext);
//GET ROUTES
void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc);
//DELETE ROUTES
void routeRequestDELETE(HttpRequest &req, HttpResponse &res, ServerContext serverContext);
//CGI ROUTES
void routeRequestCGI(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
//POST CONTROLLER
void handleLogin(HttpRequest &req, HttpResponse &res);
void handleCreateAccount(HttpRequest &req, HttpResponse &res);
void uploadFile(HttpRequest &req, HttpResponse &res);
//GET CONTROLLER
void handleGetFile(HttpRequest &req, HttpResponse &res);
void getFileNames(HttpRequest &req, HttpResponse &res);
void checkRootPassword(HttpRequest &req, HttpResponse &res);
void getBigMessage(HttpRequest &req, HttpResponse &res);
//DELETE CONTROLLER
void delteFile(HttpRequest &req, HttpResponse &res);
//CGI CONTROLLER
void handleLs(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
void handleLoop(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
void executeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
//PAGES
void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage);
void handle400(HttpResponse &res);
void handle401(HttpResponse &res);
void handle403(HttpResponse &res);
void handle404(HttpResponse &res);
void handle405(HttpResponse &res);
void handle500(HttpResponse &res);
void handle501(HttpResponse &res);
void handleHome(HttpResponse &res);
void handleIndexSstyle(HttpResponse &res);
void handleIndexJs(HttpResponse &res);
void handleIndexImgJkarras(HttpResponse &res);
void handleIndexImgAtoepper(HttpResponse &res);
void handleIndexImgRmathes(HttpResponse &res);
void handleIndexImgLogo(HttpResponse &res);
void handleRemoteStorageJs(HttpResponse &res);
void handleDashboard(HttpResponse &res);
void handleDashboardStyle(HttpResponse &res);
void handleDashboardJs(HttpResponse &res);
#endif
