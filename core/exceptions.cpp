/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "exceptions.h"

const char* ebadField::what() const throw()
{
	return "The requested field doesn't exist.";
}

const char* ebadPost::what() const throw()
{
	return "The requested post doesn't exist.";
}

const char* ebadTopic::what() const throw()
{
	return "The requested topic doesn't exist.";
}

const char* ebadForum::what() const throw()
{
	return "The requested forum doesn't exist.";
}

const char* ebadUser::what() const throw()
{
	return "The requested user doesn't exist.";
}