#include <string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "database.h"
#include "post.h"
#include "exceptions.h"
#pragma once

class Forum
{
	int fid;
	std::string name;
	//static sql::PreparedStatement * getStatement;
	//static sql::PreparedStatement * updateStatement;

	// Might be tricky to cache these in a setup with multiple servers..
#ifndef SERVER_CLUSTER
	int lastPoster; 
	int lastPost;
	std::string lastPosterName;
#endif

public:
	static void prepare();
	Forum();
	Forum(int fid);
	Forum(sql::ResultSet * res);
	bool setName(std::string _name);
	std::string getName();
	int getID();
	int getLastPoster();
	bool setLastPoster(int _lastPoster);
	bool Forum::setLastPoster(int _lastPoster, std::string _lastPosterName);
	bool Forum::setLastPost(int _lastPost);
	bool Forum::setLastPost(Post _lastPost);
};

std::map<int,Forum> getAllForums();