#include <exception>
#pragma once

class ebadField : public std::exception
{
	virtual const char* what() const throw();
};

class ebadPost : public std::exception
{
	virtual const char* what() const throw();
};

class ebadForum : public std::exception
{
	virtual const char* what() const throw();
};