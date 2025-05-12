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
#define CHUNK_SIZE 50000
#define BUFFER_SIZE 50000
#define TIME_TO_KILL_CHILD 10000000000
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

enum CgiParseState{
	CGI_START_LINE,
	CGI_KEY,
	CGI_BEFORE_VALUE,
	CGI_VALUE,
	CGI_CRLN,
	CGI_HEADERS_END
};

struct HttpResponse {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;
	ResponseState state;
	size_t bodySent;
	size_t CgiBodySent;
	long long startTime;
	bool sendingBodyToCgi;
	bool readFromCgiFinished;
	bool writeError;
	bool chunked;
	std::string cgiBody;
	size_t cgiBodySize;


	HttpResponse() : version("HTTP/1.1"), chunked(false), readFromCgiFinished(false), writeError(false), CgiBodySent(0), sendingBodyToCgi(false), statusCode(200), statusMessage("OK"), state(NOT_STARTED), bodySent(0), startTime(0) {}
};

struct file {
	std::string contentType;
	std::string path;
};

struct dir {
	std::string path;
	std::vector<dir> dirs;
	std::vector<file> files;
};


struct ServerContext {
	std::map<int, HttpRequest *> requests;
	std::map<int, HttpResponse> responses;
	std::map<int, int> fds;
	std::map<int, int> cgifds;
	std::map<int, pid_t> pids;
	dir tree;
};

struct SearchResult {
	bool found;
	bool isDir;
	file foundFile;
	dir foundDir;
	SearchResult() : found(false), isDir(false) {}
};


struct errorPage {
	size_t errorCode;
	std::string path;
};

struct location {
	bool post;
	bool get;
	bool del;
	size_t client_max_body_size;
	std::string name;
	std::string root;
	std::string index;
	std::vector<std::string> cgi_paths;
	std::vector<std::string> cgi_ext;
	std::string redirect;
	dir tree;
	bool regularLocation;
	std::string ext;
	std::string pattern;
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

struct formData{
	std::string boundary;
	std::string dis;
	std::string name;
	std::string filename;
	std::string contentType;
	std::string fileContent;
};

//SERVER
void startServer(std::map<int, ConfigData> &data);
	//SERVERINIT
		bool initServerConfigTmp(std::map<int, ConfigData> &data);
		bool initLocations(std::map<int, ConfigData> &data);
	//HANDLEEVENTS
		bool addEvent(ConfigData &configData);
		bool handleEventReq(ConfigData &configData, int i);
		bool handleEventRes(ConfigData &data, int i);
		void handleCgiWrite(ConfigData &data, int i, ServerContext &srv);
		void handleCgiRead(ConfigData &data, int i, ServerContext &srv);
	//SERVERSHUTDOWN
		void closeEverything(std::map<int, ConfigData> &data);
	//EPOLLS
		void epollHup(ConfigData &configData, int i);
		void epollOut(ConfigData &configData, int i);
		void epollIn(ConfigData &configData, int i);

//HANDLE_REQUEST
void handleRequest(int clientFd, ConfigData &configData);
void handleErrorRequest(int clientFd, ConfigData &configData, HttpRequest &req);
void addResponseEpoll(server &Server, int clientFd, ConfigData &configData, HttpResponse &res);
server &getServer(int clientFd, ConfigData &configData);
HttpRequest getReq(server &Server, int clientFd);
void normelaizePaths(HttpRequest &req, server &Server);
location* matchLocation(server &Server, const std::string &path, HttpRequest &req) ;



void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage);
std::string getFileExtension(const std::string &filename);
std::string getContentType(const std::string &extension);
bool isCGIFile(const std::string &fileName, const std::vector<std::string> &cgi_ext);
bool findInDirTree(const dir &current, const std::string &targetPath, SearchResult &result);//CONFIC

bool parseConfic(std::string path, std::map<int, ConfigData> &data);
void printAll(std::map<int, ConfigData> &data);

//HTTPPARSER
void printHttpRequest(const HttpRequest& request);
void parseHttpRequest(ConfigData &config, int client_fd, std::string &data);




//CGI
	int parseCgiContent(HttpResponse &res);

//SIG
void handle_sigint(int sig, siginfo_t *siginfo, void *context);
bool initSignal(void);
//HANDLE REQ

//helper
bool setsetExecutable(std::string &filePath);
void redirectOutfile(std::string content, std::string filePath, size_t length, size_t bytesRead);
std::string toStringInt(int number);
std::string sizeTToHex(size_t number);
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
void routeRequestPOST(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd);
//GET ROUTES
void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd);
//DELETE ROUTES
void routeRequestDELETE(HttpRequest &req, HttpResponse &res, server &server, location &loc);
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
void executeSkript(HttpRequest &req, HttpResponse &res, server &server, int clientFd, file f);
//PAGES
void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage);
void handle400(HttpResponse &res);
void handle401(HttpResponse &res);
void handle403(HttpResponse &res);
void handle504(HttpResponse &res);
void handle502(HttpResponse &res);
void handle404(HttpResponse &res);
void handle413(HttpResponse &res);
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
