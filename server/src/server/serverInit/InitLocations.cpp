#include "../../../include/webserv.hpp"

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
