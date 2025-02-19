/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 12:20:19 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/19 12:02:28 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int main(int argc, char **argv) {

	ConficData data;

	if (argc > 2) {
		std::cout << "Bad arg num\n";
		return (1);
	} else if (argc == 2) {

		if (parseConfic(std::string(argv[1]), &data)) {
			return (1);
		} else {
			if(!initSignal()) {
				return (1);
			}
			startServerWithConfic(data);
		}
	} else {
		if(!initSignal()) {
			return (1);
		}
		startServer();
	}
	return (0);
}
