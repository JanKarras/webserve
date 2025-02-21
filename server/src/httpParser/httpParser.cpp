/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atoepper <atoepper@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 12:15:23 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/21 13:56:49 by atoepper         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void printHttpRequest(const HttpRequest& request) {
    std::cout << "===== HTTP REQUEST =====" << std::endl;
    std::cout << "Method: " << request.method << std::endl;
    std::cout << "URI: " << request.uri << std::endl;
    std::cout << "Version: " << request.version << std::endl;
    std::cout << std::endl << "--- HEADERS ---" << std::endl;

    // Map durchlaufen mit einem klassischen Iterator
    std::map<std::string, std::string>::const_iterator it;
    for (it = request.headers.begin(); it != request.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    std::cout << std::endl << "--- BODY ---" << std::endl;
    if (!request.body.empty()) {
        std::cout << request.body << std::endl;
    } else {
        std::cout << "(No Body)" << std::endl;
    }

    std::cout << std::endl << "State: " << request.state << std::endl;
    std::cout << "========================" << std::endl;
}

/* void parseHttpRequest(HttpRequest &req, std::string &buffer) {
	std::istringstream stream(buffer);
	std::string line;

	while (std::getline(stream, line) && req.state != COMPLETE) {
		if (req.state == REQUEST_LINE) {
			std::istringstream requestLine(line);
			requestLine >> req.method >> req.uri >> req.version;
			req.state = HEADERS;
		} else if (req.state == HEADERS) {
			if (line == "\r") {
				if (req.headers.count("Content-Length")) {
					req.state = BODY;
				} else {
					req.state = COMPLETE;
				}
			} else {
				size_t pos = line.find(":");
				if (pos != std::string::npos) {
					std::string key = line.substr(0, pos);
					std::string value = line.substr(pos + 2);
					req.headers[key] = value;
				}
			}
		} else if (req.state == BODY) {
			req.body += line;
			if (req.body.size() >= strtoul(req.headers["Content-Length"].c_str(), NULL, 10)) {
				req.state = COMPLETE;
			}
		}
	}
} */
