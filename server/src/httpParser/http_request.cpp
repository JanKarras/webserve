#include "../include/webserv.hpp"

int parseHttpRequestLine(HttpRequest &req)
{
	size_t p = req.pos;
	RequestLineState state = static_cast<RequestLineState>(req.parseState);
	
	size_t buffer_length = req.buffer.length();
	
	for (p; p < buffer_length; p++)
	{
		switch (state)
		{
			case START:

			case METHOD:

			case URI:

			case VERSION:

			case DONE:

		}
	}
	req.pos = p;
	req.parseState = static_cast<int>(state);
	if (state == DONE)
	{
		req.parseState = 0;
		req.state = HEADERS;
		/* clear buffer, set req.pos to 0 */
		return 0;
	}
	req.state = REQUEST_LINE;
	return SUCCESS;
}

int parseHttpHeaderLine(HttpRequest &req)
{
	size_t p = req.pos;
	size_t buffer_length = req.buffer.length();
	
	while (req.state != COMPLETE && !req.buffer.empty()) {
		std::istringstream stream(req.buffer);  // Create a new stream with the updated buffer
		std::string line;

		if (std::getline(stream, line)) {
			if (line == "\r") {
				if (req.headers.count("Content-Length")) {
					req.state = BODY;
				} else {
					req.state = COMPLETE;
				}
			} else {
				size_t splitPos = line.find(":");
				if (splitPos != std::string::npos) {
					std::string key = line.substr(0, splitPos);
					std::string value = line.substr(splitPos + 2);
					req.headers[key] = value;
				}
			}
			req.buffer.erase(0, line.size() + 1);  // Erase the processed line and newline
		} else {
			break; // In case getline fails (empty buffer)
		}
	}
	return 0; // or appropriate return value
}

/* parse URI? */

int parseHttpBody(HttpRequest &req)
{
	size_t p = req.pos;
	size_t buffer_length = req.buffer.length();

	req.body.append(req.buffer);
	if (req.body.size() >= strtoul(req.headers["Content-Length"].c_str(), NULL, 10))
	{
		req.buffer.clear();
		req.state = COMPLETE;
	}
}

void	parseHttpRequest(HttpRequest &req, std::string &data)
{
	req.buffer.append(data);
	switch (req.state)
	{
		case REQUEST_LINE:
			parseHttpRequestLine(req);
		case HEADERS:
			parseHttpHeaderLine(req);
		case BODY:
			parseHttpBody(req);
	}
}
