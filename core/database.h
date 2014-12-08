#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#pragma once

class ForoDatabase
{
public:
	sql::Connection * con;
	std::string prefix;

	ForoDatabase(sql::Connection * _con);
	ForoDatabase(sql::Connection * _con, std::string prefix);
};