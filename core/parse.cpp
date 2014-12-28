#include "stdafx.h"
#include "parse.h"

bool validatePathStrict(std::string str)
{
	std::string::iterator it;
	//char lastChar;
	for (it = str.begin() + 1; it != str.end(); ++it)
	{
		//if (!isalnum(*it) && (*it != '.' || lastChar == '.')) return "";
		// Only allow alpha-numeric characters for security reasons..
		if (!isalnum(*it))
		{
			if (*it == '.') break;
			return false;
		}
		//lastChar = *it; // We don't want someone trying to break out of the standard path with ..
	}
	return true;
}

std::string sanitisePathStrict(std::string str)
{
	std::string::iterator it;
	//char lastChar;
	for (it = str.begin() + 1; it != str.end(); ++it)
	{
		// Stop iterating through the characters when something dangerous is found..
		if (!isalnum(*it))
		{
			int index = std::distance(str.begin(), it);
			return str.substr(0, index);
		}
	}
	return str;
}

std::string sanitisePathLoose(std::string str)
{
	std::string::iterator it;
	char lastChar;
	for (it = str.begin() + 1; it != str.end(); ++it)
	{
		// Only allow alpha-numeric characters for security reasons..
		if (!isalnum(*it))
		{
			if (*it == '.' && lastChar == '.') return "";

			if (*it != '-' && *it != '.')
			{
				int index = std::distance(str.begin(), it);
				return str.substr(0, index);
			}
		}
		lastChar = *it; // We don't want someone trying to break out of the standard path with ..
	}
	return str;
}

std::map<std::string, std::string> parseQueryString(std::string queryString)
{
	std::string::iterator it;
	std::map<std::string,std::string> items;
	// TO-DO: Add a compile time switch that trades memory for performance.
	// OR
	// TO-DO: Use a Regex for this to let the compiler deal with the details.
	std::string name = "";
	std::string data = "";
	name.reserve(20); // This should be enough space to cover most of the field names..
	data.reserve(50); // This should be enough space to cover most of the field data..

	bool is_name = true;
	for (it = queryString.begin(); it != queryString.end(); ++it)
	{
		// Unfortunately, we have to do a layer of sanitisation here to make sure that nothing suspicious has been snuck in..
		if (!isalnum(*it) && *it != '=' && *it != '&' && *it != '+') continue;
		
		if (is_name)
		{
			if (*it == '=') is_name = false;
			else name += *it;
		}
		else
		{
			if (*it == '&')
			{
				is_name = true;
				items[name] = data;

				// Empty these temporary variables..
				name = "";
				data = "";
			}
			else data += *it;
		}
	}

	// Deal with the last item..
	if (!is_name) items[name] = data;

	return items;
}