#include "stdafx.h"
#include "database.h"

ForoDatabase::ForoDatabase(sql::Connection * _con)
{
	con = _con;
	prefix = "";
}

ForoDatabase::ForoDatabase(sql::Connection * _con, std::string _prefix)
{
	con = _con;
	prefix = _prefix;
}