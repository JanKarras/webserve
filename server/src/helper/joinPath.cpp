#include "../../include/webserv.hpp"

std::string joinPath(const std::string &base, const std::string &suffix) {
    if (base.empty()) {
		return suffix;
	}
    if (suffix.empty()) {
		return base;
	}
    if (base[base.size() - 1] == '/' && suffix[0] == '/') {
        return base + suffix.substr(1);
	}
    else if (base[base.size() - 1] != '/' && suffix[0] != '/') {
        return base + "/" + suffix;
	} else {
        return base + suffix;
	}
}
