/* --- Harl.cpp --- */

/* ------------------------------------------
author: undefined
date: 11/7/2024
------------------------------------------ */

#include "../header/Harl.h"

Harl::Harl(std::string level) {
	std::string levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
	for (int i = 0; i < 4; i++)
	{
		if (levels[i] == level) {
			this->_level = i;
			return ;
		}
	}
	_level = -1;
}

Harl::~Harl() {
    // Destructor
}

void	Harl::debug( void ) {
	std::cout << "[DEBUG]\n";
	std::cout << "I love having extra bacon for my";
	std::cout << "7XL-double-cheese-triple-pickle-specialketchup burger.";
	std::cout << "I really do!\n\n";
}

void	Harl::info( void ) {
	std::cout << "[INFO]\n";
	std::cout << "I cannot believe adding extra bacon costs more money. You didn’t put";
	std::cout << "enough bacon in my burger! If you did, I wouldn’t be asking for more!\n\n";
}

void	Harl::warning( void ) {
	std::cout << "[WARNING]\n";
	std::cout << "I think I deserve to have some extra bacon for free. I’ve been coming for";
	std::cout << "years whereas you started working here since last month.\n\n";
}

void	Harl::error( void ) {
	std::cout << "[ERROR]\n";
	std::cout << "This is unacceptable! I want to speak to the manager now.\n\n";
}


int		getCase(std::string level) {
	std::string	levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
	for (int j = 0; j < 4; j++)
	{
		if (levels[j] == level)
			return (j);
	}
	return (4);
}

void	Harl::complain(std::string level) {
	void		(Harl::*infoAction)() = &Harl::info;
	void		(Harl::*debugAction)() = &Harl::debug;
	void		(Harl::*warningAction)() = &Harl::warning;
	void		(Harl::*errorAction)() = &Harl::error;
	void		(Harl::*actions[])() = {debugAction, infoAction, warningAction, errorAction};
	int			levelNb = getCase(level);
	switch (levelNb) {
		case 0:
			if (levelNb >= this->_level)
				(this->*actions[0])();
			break;
		case 1:
			if (levelNb >= this->_level)
				(this->*actions[1])();
			break;
		case 2:
			if (levelNb >= this->_level)
				(this->*actions[2])();
			break;
		case 3:
			if (levelNb >= this->_level)
				(this->*actions[3])();
			break;
		case 4:
			std::cout << "unknown level\n";
			break;
		default:
			break;
	}
}

int	Harl::getLevel( void ) {
	return (this->_level);
}
