#include "../../include/webserv.hpp"

// void startServer(ConfigData &conficData, bool conficFlag) {
// 	ServerContext ServerContext;
// 	ServerContext.serverFd = -1;
// 	ServerContext.epollFd = -1;

// 	struct sockaddr_in serverAddress;
// 	struct epoll_event event, events[MAX_EVENTS];
// 	if (conficFlag) {
// 		if (!initServerConfic(ServerContext, serverAddress, event, conficData)) {
// 			return ;
// 		}
// 	} else {
// 		if (!initServer(ServerContext, serverAddress, event)) {
// 			return ;
// 		}
// 	}

// 	initRoutes(ServerContext);

// 	while (running) {
// 		int numEvents = epoll_wait(ServerContext.epollFd, events, MAX_EVENTS, -1);
// 		if (numEvents == -1) {
// 			std::cerr << "Failed to wait for events." << std::endl;
// 			break;
// 		}
// 		for (int i = 0; i < numEvents; ++i) {
// 			if (events[i].data.fd == ServerContext.serverFd) {
// 				if (!addEvent(ServerContext, event)) {
// 					continue;
// 				}
// 			} else if (events[i].events & EPOLLIN) {
// 				if (!handleEventReq(ServerContext, events, i)) {
// 					continue;
// 				}
// 			} else if (events[i].events & EPOLLOUT) {
// 				if (!handleEventRes(ServerContext, events, i)) {
// 					continue;
// 				}
// 			}
// 		}
// 	}

// 	std::cout << "test\n";

// 	closeAll(ServerContext);
// }

static int parseCgiContent(HttpResponse &res)
{
	std::string		key, value;
	size_t			pos = 0;
	uint8_t			ch;
	CgiParseState	state = CGI_START_LINE;
	
	for (; pos < res.body.size(); pos++)
	{
		ch = res.body[pos];
		switch(state)
		{
			case(CGI_START_LINE):
				key.erase();
				value.erase();
				if (ch == '\r')
					state = CGI_HEADERS_END;
				else if (isalnum(ch))
				{
					key.push_back(ch);
					state = CGI_KEY;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;
				}
				break;
			case(CGI_KEY):
				if (ch == ':')
					state = CGI_BEFORE_VALUE;
				else if (!isalnum(ch) && ch != '-')
				{
					Logger::error("Malformed header key from cgi script");
					return FAILURE;
				}
				else
					key.push_back(ch);
				break;
			case(CGI_BEFORE_VALUE):
				if (ch == ' ')
					break;
				else if (ch == '\r')
					state = CGI_CRLN;
				else if (ch >= 32 && ch <= 126)
				{
					value.push_back(ch);
					state = CGI_VALUE;
				}
				else
				{
					Logger::error("Malformed header value from cgi script");
					return FAILURE;					
				}
				break;
			case(CGI_VALUE):
				if (ch == '\r')
					state = CGI_CRLN;
				else if (ch >= 32 && ch <= 126)
				{
					value.push_back(ch);
					state = CGI_VALUE;
				}
				else
				{
					Logger::error("Malformed header value from cgi script");
					return FAILURE;					
				}
				break;
			case(CGI_CRLN):
				if (ch == '\n')
				{
					res.headers[key] = value;
					state = CGI_START_LINE;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;	
				}
				break;
			case(CGI_HEADERS_END):
				if (ch == '\n')
				{
					res.body.erase(0, pos + 1);
					return SUCCESS;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;	
				}
				break;	
		}
	}
	Logger::error("Unexpected EOF in CGI headers");
	return FAILURE;
}

bool initServerConfigTmp(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		ConfigData &configData = it->second;

		configData.serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (configData.serverFd == -1) {
			Logger::error("Failed to create socket on port %i", configData.port);
			return (false);
		}

		configData.serverAddress.sin_family = AF_INET;
		configData.serverAddress.sin_addr.s_addr = INADDR_ANY;
		configData.serverAddress.sin_port = htons(configData.port);

		if (LOOP != 1) {
			if (bind(configData.serverFd, (struct sockaddr*)&configData.serverAddress, sizeof(configData.serverAddress)) == -1) {
				Logger::error("Failed to bind socket.");
				close(configData.serverFd);
				return (false);
			}
		} else {
			while (bind(configData.serverFd, (struct sockaddr*)&configData.serverAddress, sizeof(configData.serverAddress)) == -1) {
				Logger::error ("bind faild, Retry in 2 secs");
				sleep(2);
			}
		}

		if (listen(configData.serverFd, MAX_CLIENTS) == -1){
			Logger::error("Failed to listen.");
			close(configData.serverFd);
			return (false);
		}

		configData.epollFd = epoll_create1(0);
		if (configData.epollFd == -1) {
			Logger::error("Failed to create epoll instance.");
			close(configData.serverFd);
			return (false);
		}

		configData.event.events = EPOLLIN;
		configData.event.data.fd = configData.serverFd;
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, configData.serverFd, &configData.event) == -1) {
			Logger::error("Failed to add server socket to epoll instance.");
			close(configData.serverFd);
			close(configData.epollFd);
			return (false);
		}

		Logger::info("Server started and liestening on port: %i", configData.port);
	}

	return true;
}

bool buildDirTree(const std::string &directoryPath, dir &currentDir) {
    DIR *dirPtr;
    struct dirent *entry;
    struct stat pathStat;

    currentDir.path = directoryPath;

    dirPtr = opendir(directoryPath.c_str());
    if (dirPtr == NULL) {
        std::cerr << "Cannot open directory: " << directoryPath << std::endl;
        return false;
    }

    while ((entry = readdir(dirPtr)) != NULL) {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        std::string fullPath = directoryPath;
        if (fullPath.length() == 0 || fullPath[fullPath.length() - 1] != '/')
            fullPath += "/";
        fullPath += name;

        if (stat(fullPath.c_str(), &pathStat) == -1) {
            std::cerr << "Cannot access path: " << fullPath << std::endl;
            continue;
        }

        if (S_ISDIR(pathStat.st_mode)) {
            dir subDir;
            if (buildDirTree(fullPath, subDir)) {
                currentDir.dirs.push_back(subDir);
            }
        } else if (S_ISREG(pathStat.st_mode)) {
            file f;
            f.path = fullPath;
            f.contentType = getContentType(getFileExtension(fullPath));
            currentDir.files.push_back(f);
        }
    }

    closedir(dirPtr);
    return true;
}


std::string getContentType(const std::string &extension) {
	static std::map<std::string, std::string> mimeTypes;

	if (mimeTypes.empty()) {
		mimeTypes[".html"] = "text/html";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".json"] = "application/json";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".wav"] = "audio/wav";
		mimeTypes[".zip"] = "application/zip";
		mimeTypes[".sh"] = "application/x-sh";
		mimeTypes[".py"] = "text/x-python";
	}

	std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
	if (it != mimeTypes.end()) {
		return it->second;
	} else {
		return ("application/octet-stream");
	}
}

std::string getFileExtension(const std::string &filename) {
	size_t dotPos = filename.find_last_of(".");
	if (dotPos != std::string::npos) {
		return filename.substr(dotPos);
	}
	return "";
}

bool initLocations(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		ConfigData &confgData = it->second;
		for (size_t i = 0; i < confgData.servers.size(); i++) {
			server &Server = confgData.servers[i];

			buildDirTree(Server.root, Server.serverContex.tree);

			for (size_t j = 0; j < Server.locations.size(); ++j) {
				if (!Server.locations[j].root.empty()) {
					buildDirTree(Server.locations[j].root, Server.locations[j].tree);
				}
			}

		}

	}
	return (true);
}

void handleCgiRead(ConfigData &data, int i, ServerContext &srv) {
	int cgifd = data.events[i].data.fd;
	int clientFd = 0;


	for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); ++it) {
		if (it->second == cgifd) {
			clientFd = it->first;
		}
	}
	std::map<int, HttpResponse>::iterator itt = srv.responses.find(clientFd);
	HttpResponse &res = itt->second;

	char buffer[BUFFER_SIZE];

	ssize_t bytesRead = read(cgifd, buffer, CHUNK_SIZE);
	Logger::debug("Bytes read from cgi: %i", bytesRead);
	if (bytesRead == -1) {
		return;
	} else if (bytesRead == 0) {
		res.readFromCgiFinished = true;
		close(cgifd);
		srv.fds.erase(clientFd);
		res.state = SENDING_HEADERS;
	} else {
		res.body.append(buffer, bytesRead);
	}

}

void handleCgiWrite(ConfigData &data, int i, ServerContext &srv) {
	int cgifd = data.events[i].data.fd;
	int clientFd= 0;
	for (std::map<int, int>::iterator it = srv.cgifds.begin(); it != srv.cgifds.end(); ++it) {
		if (it->second == cgifd) {
			clientFd = it->first;
		}
	}
	std::map<int, HttpResponse>::iterator itt = srv.responses.find(clientFd);
	HttpResponse &res = itt->second;

	size_t newBodySize = res.cgiBody.size();
	if (res.cgiBody.size() > CHUNK_SIZE) {
		newBodySize = CHUNK_SIZE;
	}
	std::string sendString = res.cgiBody.substr(0, newBodySize);
	ssize_t bytesSent = write(cgifd, sendString.c_str(), newBodySize);

	Logger::debug("Bytes send to cgi: %i", bytesSent);
	if (bytesSent == -1) {
		return;
	} else {
		res.cgiBody.erase(0, bytesSent);
	}
	if (res.cgiBody.size() != 0) {
		return;
	}
	Logger::debug("All body data sent to CGI for clientFd: %d", clientFd);
	close(cgifd);
	srv.cgifds.erase(clientFd);
	res.sendingBodyToCgi = false;
}

void startServer(std::map<int, ConfigData> &data) {



	if(!initServerConfigTmp(data)) {
		Logger::error("Server init doesn't finished successfully");
		return ;
	}
	Logger::info("Server init finished successfully");

	if(!initLocations(data)) {
		Logger::error("Locations init doesn't finished successfully");
		return ;
	}

	//printAll(data);

	while (running) {
		for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
			ConfigData &configData = it->second;
			int numEvents = epoll_wait(configData.epollFd, configData.events, MAX_EVENTS, 1);
			if (numEvents == -1) {
				Logger::error("Failed to wait for events.");
				break;
			}
			for (int i = 0; i < numEvents; ++i) {
				if (configData.events[i].data.fd == configData.serverFd) {
					//Logger::info("New client connection detected.");
					if (!addEvent(configData)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLIN) {
					bool tnp = false;
					for (size_t j = 0; j < configData.servers.size(); j++) {
						ServerContext &srv = configData.servers[j].serverContex;
						for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); it++) {
							if (it->second == configData.events[i].data.fd) {
								handleCgiRead(configData, i, srv);
								tnp = true;
								break;
							}
						}
						// if (srv.fds.find(configData.events[i].data.fd) != srv.fds.end()) {
						// 	handleCgiRead(configData, i, srv);
						// 	tnp = true;
						// 	break;
						// }
					}
					if (tnp) {
						continue;
					}
					//Logger::info("EPOLLIN event for fd: %d", configData.events[i].data.fd);
					if (!handleEventReq(configData, i)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLOUT) {
					bool tnp = false;
					for (size_t j = 0; j < configData.servers.size(); j++) {
						ServerContext &srv = configData.servers[j].serverContex;
						for (std::map<int, int>::iterator it = srv.cgifds.begin(); it != srv.cgifds.end(); it++) {
							if (it->second == configData.events[i].data.fd) {
								handleCgiWrite(configData, i, srv);
								tnp = true;
								break;
							}
						}
					}
					if (tnp) {
						continue;
					}
					if (!handleEventRes(configData, i)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLHUP) {
					for (size_t j = 0; j < configData.servers.size(); ++j) {
						ServerContext &srv = configData.servers[j].serverContex;
						for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); ++it) {
							if (it->second == configData.events[i].data.fd) {
								int clientFd = it->first;
								HttpResponse &res = srv.responses[clientFd];

								close(configData.events[i].data.fd);
								srv.fds.erase(clientFd);
								res.readFromCgiFinished = true;
								res.state = SENDING_HEADERS;
								/*parse cgi content
								parseCgiContent(res);
								*/
								Logger::debug("EPOLLHUP: CGI closed pipe, clientFd = %d", clientFd);
								break;
							}
						}
					}
					continue;
				}
			}
		}
	}
}
