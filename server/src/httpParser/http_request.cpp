#include "../../include/webserv.hpp"

static void setRequestError(HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = HTTP_BAD_REQUEST;
}

int parseHttpRequestLine(HttpRequest &req)
{
	size_t p = req.pos;
	size_t uriLength;
	RequestLineState state = static_cast<RequestLineState>(req.parseState);
	
	size_t buffer_length = req.buffer.length();
	
	for (p; p < buffer_length; p++)
	{
		u_char c = req.buffer[p];
		switch (state)
		{
			case RL_START:
				if (c == CR || c == LF)
					break;
				if (c < 'A' || c > 'Z')
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				state = RL_METHOD;
				break;
			case RL_METHOD:
				if (c == ' ')
				{
					/* check method with strcmp */
					if (!req.buffer.compare(0, p, "GET "))
						req.method = GET;
					else if (!req.buffer.compare(0, p, "POST "))
						req.method = POST;
					else if (!req.buffer.compare(0, p, "DELETE "))
						req.method = DELETE;
					else
					{
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}	
					state = RL_URI;
				}
				if (c < 'A' || c > 'Z' || p > 5)
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case RL_URI:
				uriLength = req.buffer.find(' ', p + 1);
				if (uriLength != std::string::npos)
				{
					req.uri = req.buffer.substr(p + 1, uriLength);
					/* parse URI */
					p += uriLength;
					state = RL_VERSION;
				}
				break;
			case RL_VERSION:

				if (buffer_length - p >= 10)
					if (!req.buffer.compare(p + 1, 10, "HTTP/1.1\r\n"))
					{
						req.version = "HTTP/1.1";
						p += 10;
						state = RL_DONE;
					}
					else 
					{
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
				break;
		}
	}
	req.pos = p;
	req.parseState = static_cast<int>(state);
	if (state == RL_DONE)
	{
		req.parseState = 0;
		req.state = HEADERS;
		req.buffer.erase(0, p + 1);
		req.pos = 0;
		return SUCCESS;
	}
	req.state = REQUEST_LINE;
	return SUCCESS;
}

int parseHttpHeaderLine(HttpRequest &req)
{
	size_t p;
	while (!req.buffer.empty() && req.state != COMPLETE) {
		p = req.buffer.find("\r\n");
		if (p == std::string::npos)
			break;

		std::string line = req.buffer.substr(0, p);
		req.buffer.erase(0, p + 2);  // Remove line + CRLF

		if (line.empty()) {
			req.state = req.headers.count("Content-Length") ? BODY : COMPLETE;
			break;
		}

		size_t splitPos = line.find(":");
		if (splitPos != std::string::npos) {
			std::string key = line.substr(0, splitPos);
			std::string value = line.substr(splitPos + 2);
			req.headers[key] = value;
		}
	}
	return 0;
}

/* parse URI ... */

int parseHttpBody(HttpRequest &req)
{
	req.body.append(req.buffer);
	req.buffer.clear();

	if (req.headers.count("Content-Length")) 
	{
		char *endptr = NULL;
		unsigned long contentLength = strtoul(req.headers["Content-Length"].c_str(), &endptr, 10);

		// Check if conversion was successful
		if (*endptr != '\0') {
			setRequestError(req, HTTP_BAD_REQUEST);
			return FAILURE;
		}

		if (req.body.size() >= contentLength) 
		{
			req.state = COMPLETE;
		}
	}
	return SUCCESS;
}

void	parseHttpRequest(HttpRequest &req, std::string &data)
{
	req.buffer.append(data);
	switch (req.state)
	{
		case REQUEST_LINE:
			parseHttpRequestLine(req);
			if (req.state != HEADERS)
				break;
		case HEADERS:
			parseHttpHeaderLine(req);
			if (req.state != BODY)
				break;
		case BODY:
			parseHttpBody(req);
			break;
	}
}
