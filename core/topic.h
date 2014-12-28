/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "database.h"
#include "exceptions.h"
#pragma once

extern ForoDatabase * db;

class Topic
{
public:
	int tid = 0;
	std::string name = "";
	int author = 0;
	bool is_locked = false;
	int fid = 0;

	Topic();
	Topic(int _tid);
	Topic(sql::ResultSet * res);
	static void Topic::prepare();

	int getID();

	int getFID();
	bool setFID(int _fid);

	std::string getName();
	bool setName(std::string _name);
	int getAuthor();
	bool setAuthor(int _author);
	bool isLocked();
	bool setLocked(bool _locked);

	sql::ResultSet * getPosts();
	// Post[] getPostSet();
};