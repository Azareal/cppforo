#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>

class Database
{
public:
	sql::Connection * con;
	std::string prefix;

	Database(sql::Connection * _con);
	Database(sql::Connection * _con, std::string prefix);
};