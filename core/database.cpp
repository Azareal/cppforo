#include "stdafx.h"
#include "database.h"

Database::Database(sql::Connection * _con)
{
	con = _con;
	prefix = "";
}

Database::Database(sql::Connection * _con, std::string _prefix)
{
	con = _con;
	prefix = _prefix;
}