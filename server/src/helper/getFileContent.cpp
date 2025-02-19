/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getFileContent.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 11:04:02 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/19 11:10:21 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"


std::string getFileContent(std::string filePath) {
	std::string publicPath = "../../../";
	publicPath.append(filePath);
	std::ifstream file(publicPath.c_str());

	std::string body;

	if (file) {
		std::string line;
		while (getline(file, line)) {
			body+= line + "\n";
		}
		file.close();
	} else {
		std::cerr << "File not found\n";
	}
	return body;
}
