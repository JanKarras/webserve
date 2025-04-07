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

void listFilesRecursive(const std::string &directory, std::vector<std::string> &files) {
    DIR *dir;
    struct dirent *entry;
    struct stat pathStat;

    if ((dir = opendir(directory.c_str())) == NULL) {
        std::cerr << "Error: Cannot open directory " << directory << std::endl;
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string fullPath = directory;
		if (fullPath[fullPath.size() - 1] != '/') {  // Falls kein / am Ende, füge es hinzu
    		fullPath += "/";
		}
		fullPath += entry->d_name;


        if (stat(fullPath.c_str(), &pathStat) == -1) {
            std::cerr << "Error: Cannot access " << fullPath << std::endl;
            continue;
        }

        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue;
        }

		std::cout << "filesINIT for " << directory << std::endl;

        if (S_ISDIR(pathStat.st_mode)) {
            listFilesRecursive(fullPath, files);
        } else if (S_ISREG(pathStat.st_mode)) {
			std::cout << fullPath << std::endl;
			if (!fullPath.empty() && fullPath[0] == '/') {
                fullPath = fullPath.substr(1);
            }
            files.push_back(fullPath);
        }
    }

    closedir(dir);
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
			std::vector<std::string> files;

			listFilesRecursive(Server.root, files);

			for (size_t j = 0; j < files.size(); j++) {
				file newFile;
				newFile.path = files[j];
				newFile.contentType = getContentType(getFileExtension(files[j]));
				Server.serverContex.files.push_back(newFile);
			}

			std::vector<std::string> LocationFiles;

			for (size_t i = 0; i < Server.locations.size(); i++) {
				if (Server.locations[i].root.size() != 0) {
					listFilesRecursive(Server.locations[i].root, LocationFiles);

					for (size_t j = 0; j < LocationFiles.size(); j++) {
						file newFile;
						newFile.path = LocationFiles[j];
						newFile.contentType = getContentType(getFileExtension(LocationFiles[j]));
						Server.locations[i].files.push_back(newFile);
					}
				}
			}

		}

	}
	return (true);
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
					Logger::info("New client connection detected.");
					if (!addEvent(configData)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLIN) {
					Logger::info("EPOLLIN event for fd: %d", configData.events[i].data.fd);
					if (!handleEventReq(configData, i)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLOUT) {
					Logger::info("EPOLLOUT event for fd: %d", configData.events[i].data.fd);
					if (!handleEventRes(configData, i)) {
						continue;
					}
				}
			}
		}
	}
}
