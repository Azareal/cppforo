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
#include "exceptions.h"
#include "topic.h"
#pragma once

extern ForoDatabase * db;
class Post
{
	//static sql::PreparedStatement * getStatement;
	//static sql::PreparedStatement * updateStatement;

	int pid;
	int tid;
	std::string content;
	int author = 0;
	bool is_locked = false;
	bool is_sticky = false;
public:
	static void prepare();

	Post(int _pid);
	int getID();

	int getTID();
	bool setTID(int _tid);
	std::string getContent();
	bool setContent(std::string _content);
	int getAuthor();
	bool setAuthor(int _author);
	bool is(std::string name);
	bool lock(bool state);
	bool stick(bool state);

	Topic getParentTopic();
};
