#include "stdafx.h"
#include "logger.h"

// Outputs to the server console..
void log(std::string str)
{
	std::cout << str << std::endl;
}

void error(std::string str)
{
	std::cerr << str << std::endl;

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);
}

void error(char * str)
{
	std::cerr << str << std::endl;

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);
}