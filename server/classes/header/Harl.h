/* --- Harl.h --- */

/* ------------------------------------------
Author: undefined
Date: 11/7/2024
------------------------------------------ */

#ifndef HARL_H
#define HARL_H

#include "../../include/webserv.hpp"

class Harl {
public:
    Harl(std::string level);
    ~Harl();
	void	complain(std::string level);
	int		getLevel( void );
private:
	void	debug( void );
	void	info( void );
	void	warning( void );
	void	error( void );
	int		_level;
};

#endif // HARL_H
