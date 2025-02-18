/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleErrorRequest.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 14:33:09 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 14:40:07 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void handleErrorRequest(int clientFd, ServerContext &ServerContext) {

	HttpRequest req = ServerContext.requests[clientFd];
	HttpResponse &res = ServerContext.responses[clientFd];


	res.statusCode = req.exitStatus;

	res.version = req.version;
	res.state = SENDING_HEADERS;

	switch (res.statusCode)
	{
	case HTTP_BAD_REQUEST:
	res.statusMessage = "Http bad request";
	//res.body = getErrorFile(HTTP_BAD_REQUEST);

	res.state = SENDING_HEADERS;
		break;

	default:
		break;
	}
}
