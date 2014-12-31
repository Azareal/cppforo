/*
	Copyright (c) Azareal 2014.
	Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "user.h"

ebadUser badUser;
sql::PreparedStatement * getUserStmt;
extern ForoDatabase * db;

User::User()
{
	uid = 0;
	username = "";
	gid = 0;
	is_admin = false;
	is_mod = false;
	is_banned = false;
	session = "";
}

User::User(int _uid)
{
	// TO-DO: Store the most recent users in a map and select from there first before hitting the database..
	// This should handle guest posts.. E.g. Deleted users..
	if (_uid == 0)
	{
		username = "Guest";
		return;
	}

	sql::ResultSet * res;
	getUserStmt->setInt(1, _uid);
	res = getUserStmt->executeQuery();

	if (res->rowsCount() == 0)
	{
		throw badUser;
		return;
	}

	// Loop over the retrieved item..
	uid = _uid;
	while (res->next())
	{
		username = res->getString("username");
		gid = res->getInt("gid");
		is_admin = res->getBoolean("is_admin");
		is_mod = res->getBoolean("is_mod");
		is_banned = res->getBoolean("is_banned");
		session = res->getString("session");
	}

	delete res;
}

void User::prepare()
{
	try
	{
		getUserStmt = db->con->prepareStatement("SELECT * FROM " + db->prefix + "users WHERE uid = ? LIMIT 1");
	}
	catch (std::exception& e)
	{
		ferror(e.what());
	}
}

int User::getID()
{
	return uid;
}

std::string User::getName()
{
	return username;
}

int User::getGID()
{
	return gid;
}

bool User::loggedIn()
{
	return (uid != 0);
}

std::string User::getSession()
{
	return session;
}

User guest()
{
	User data;
	data.username = "Guest";
	return data;
}