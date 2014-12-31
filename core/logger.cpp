/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "logger.h"

// Outputs to the server console..
void log(std::string str)
{
	std::cout << str << std::endl;
}

/*void log(char * str)
{
	std::cout << str << std::endl;
}*/

void ferror(std::string str)
{
	std::cerr << str << std::endl;

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);
}

void ferror(char * str)
{
	std::cerr << std::string(str) << std::endl;

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);
}