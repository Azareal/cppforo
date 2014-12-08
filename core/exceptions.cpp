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

const char* ebadForum::what() const throw()
{
	return "The requested forum doesn't exist.";
}