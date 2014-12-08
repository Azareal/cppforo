#include <string>
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
class Post
{
	//static sql::PreparedStatement * getStatement;
	//static sql::PreparedStatement * updateStatement;
	
	int pid;
	std::string content;
	int author = 0;
	bool is_locked = false;
	bool is_sticky = false;
public:
	static void prepare();

	Post(int _pid);
	int getID();
	std::string getContent();
	bool setContent(std::string _content);
	int getAuthor();
	bool setAuthor(int _author);
	bool is(std::string name);
	bool Post::lock(bool state);
	bool Post::stick(bool state);
};