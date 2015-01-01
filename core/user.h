/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#pragma once
#include <string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <pion/http/request.hpp>

#include "database.h"
#include "exceptions.h"

class User
{
public:
	int uid = 0;
	std::string username = "";
	int gid = 0;
	bool is_admin = false;
	bool is_mod = false;
	bool is_banned = false;
	std::string session = "";

	User();
	User(int _uid);
	static void prepare();
	int getID();
	std::string getName();
	int getGID();
	std::string getSession();
	bool loggedIn();

	// TO-DO: Add these later..
	//Group getGroup();
	//bool setAdmin();
	//bool setMod();
	//bool setBanned();
};

User guest();
User checkSession(pion::http::request_ptr& http_request_ptr);