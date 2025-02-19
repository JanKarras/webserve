/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setNonBlocking.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:30:14 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/19 12:30:20 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get file descriptor flags." << std::endl;
		return -1;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		std::cerr << "Failed to set file descriptor to non-blocking." << std::endl;
		return -1;
	}
	return 0;
}
