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
#include <string.h>
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
#define CHUNK_SIZE 500000
#define BUFFER_SIZE 500000
#define TIME_TO_KILL_CHILD 3LL * 100000000

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
	bool chunked;
	bool readFromCgiFinished;
	bool writeError;
	size_t CgiBodySent;
	bool sendingBodyToCgi;
	int statusCode;
	std::string statusMessage;
	ResponseState state;
	size_t bodySent;
	long long startTime;
	std::map<std::string, std::string> headers;
	std::string body;
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
		std::string getFileExtension(const std::string &filename);
		std::string getContentType(const std::string &extension);
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

//HELPER
	//PRINTER
		void printHttpResponse(const HttpResponse &res);
		void printFile(const file &f);
		void printDir(const dir &d, int indent= 0);
		void printSearchResult(const SearchResult &res);
		void printErrorPage(const errorPage &e);
		void printLocation(const location &loc);
		void printServer(const server &srv);
		void printHttpRequest(const HttpRequest &req);
		void printConfigData(const ConfigData &config);
		void printServerContext(const ServerContext &ctx);
	//MAKE_EXECUTABLE
		bool setsetExecutable(std::string &filePath);
	//LOGGER_FOR_REQ_RES_IN_FILE
		void redirectOutfile(std::string content, std::string filePath, size_t length, size_t bytesRead);
	//TRANSFORMER
		std::string toStringInt(int number);
		std::string sizeTToHex(size_t number);
		std::string toString(long long number);
		int toIntString(const std::string &str);
	//GET_TIME
		long long getCurrentTime();
	//FILES
		std::string getDestPath(std::string email);
		std::string getFileContent(std::string filePath);
		std::string joinPath(const std::string &base, const std::string &suffix);
	//SET_NONBLOCKING
		int setNonBlocking(int fd);
	//FIND_IN_DIR_TREE
		bool findInDirTree(const dir &current, const std::string &targetPath, SearchResult &result);

//CGI
	void checkCgiTimeouts(ConfigData &data);
	int parseCgiContent(HttpResponse &res);
	void executeSkript(HttpRequest &req, HttpResponse &res, server &server, int clientFd, file f);

//HTTPPARSER
	void printHttpRequest(const HttpRequest& request);
	void parseHttpRequest(ConfigData &config, int client_fd, std::string &data);

//SIGNALS
	void handle_sigint(int sig, siginfo_t *siginfo, void *context);
	bool initSignal(void);

//ERROR_HANDLER
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

//GET_ROUTE
	void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd);

//POST_ROUTE
	void routeRequestPOST(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd);
	bool isCGIFile(const std::string &fileName, const std::vector<std::string> &cgi_ext);
	void handleRegularLocation(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd);

//DELETE_ROUTE
	void routeRequestDELETE(HttpRequest &req, HttpResponse &res, server &server, location &loc);

//PARSE_CONFIG
	bool parseConfig(std::string path, std::map<int, ConfigData> &data);
	bool parseServer(std::map<int, ConfigData> &data, std::string serverBlock);
	//IS_VALID
		bool isValidConfigLine(const std::string &line, const std::string &expectedKeyword, server &Server);
		bool isValidIntConfigLine(const std::string &line, const std::string &expectedKeyword, int minValue, int maxValue, server &Server);
		bool isValidErrorPageLine(const std::string &line, server &Server);
		bool isValidLocation(const std::string &line, location &Location);
	//INPUTFILE
		bool checkPath(std::string path);
		std::string readFile(std::string path);
		bool validateLines(const std::string &filetxt);
	//EXTRACT_BLOCKS
		std::vector<std::string> extractServerBlocks(std::string &file);
		std::vector<std::string> extractLocationBlocks(std::string &serverBlock);
	//CHECK_CONFIG
		bool checkConfig(std::map<int, ConfigData> &data);
#endif
