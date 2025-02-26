#include "../../include/webserv.hpp"

std::map<std::string, std::string> initMimeTypes( void ) {

	std::map<std::string, std::string> mimeTypes;

	mimeTypes["jpg"] = "image/jpeg";
	mimeTypes["png"] = "image/png";
	mimeTypes["sh"] = "application/x-sh";

	return mimeTypes;
}
