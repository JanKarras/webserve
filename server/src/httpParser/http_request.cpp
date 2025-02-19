#include "../../include/webserv.hpp"

int parseHttpRequestLine(HttpRequest &req, const std::string &data)
{
	size_t p = req.pos;
	req.buffer.append(data);
	req.content_length = req.buffer.length();

	enum {
		START,
		METHOD,
		URI,
		VERSION,
		DONE,
		ERROR
	} state;
	state = req.parseState;

	for (p; p < req.content_length; p++)
	{
		switch (state)
		{
			case START:

			case METHOD:

			case URI:

			case VERSION:

		}
	}
	req.pos = p;
	req.parseState = state;
	if (req.parseState == DONE)
	{
		req.parseState = START;
		req.state = HEADERS;
		return 0;
	}
	req.state = REQUEST_LINE;
	return SUCCESS;
}

void parseHttpHeaderLine(HttpRequest &req, const std::string &data)
{

}
