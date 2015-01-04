/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include <string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#ifndef __linux
#include <mysql_error.h>
#endif
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
public:
	int fid;
	std::string name;
	bool adminOnly = false;
	bool staffOnly = false;

	// Note: It might be tricky to cache these in a setup with multiple servers..
	// TO-DO: Run a task which occasionally polls the database for the last post every x seconds for server clusters..
	//#ifndef SERVER_CLUSTER
	int lastPoster;
	int lastPost;
	std::string lastPosterName;
	//#endif

	static void prepare();
	Forum();
	Forum(int fid);
	Forum(sql::ResultSet * res);
	Forum(sql::ResultSet * res, bool auto_run);
	bool setName(std::string _name);
	std::string getName();

	bool getStaffOnly();
	bool getAdminOnly();
	bool setStaffOnly(bool state);
	bool setAdminOnly(bool state);

	int getID();
	int getLastPost();
	int getLastPoster();
	bool setLastPoster(int _lastPoster);
	bool setLastPoster(int _lastPoster, std::string _lastPosterName);
	bool setLastPost(int _lastPost);
	bool setLastPost(Post _lastPost);

	sql::ResultSet * getTopics();
};

std::map<int,Forum> getAllForums();
