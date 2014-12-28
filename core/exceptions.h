/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

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

class ebadTopic : public std::exception
{
	virtual const char* what() const throw();
};

class ebadForum : public std::exception
{
	virtual const char* what() const throw();
};

class ebadUser : public std::exception
{
	virtual const char* what() const throw();
};