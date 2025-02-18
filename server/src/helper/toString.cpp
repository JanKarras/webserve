/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toString.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 13:07:42 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 13:07:55 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

std::string toString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string toString(long long number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}
