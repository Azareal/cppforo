/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "forum.h"

// Unfortunately, we can't do ? = ? in prepared statements, so we have loads of them here..
sql::PreparedStatement * forumGetTopics;
sql::PreparedStatement * forumGetStatement;
//sql::PreparedStatement * forumUpdateStatement;
sql::PreparedStatement * forumNameUpdateStatement;
sql::PreparedStatement * forumIsStaffUpdateStatement;
sql::PreparedStatement * forumIsAdminUpdateStatement;
sql::PreparedStatement * forumLastPostUpdateStatement;
sql::PreparedStatement * forumLastPosterUpdateStatement;
sql::PreparedStatement * forumLastPosterNameUpdateStatement;

ebadForum badForum;
extern ForoDatabase * db;
Forum::Forum()
{
	fid = 0;
	name = "";
	lastPoster = 0;
	lastPost = 0;
	lastPosterName = "";
}

Forum::Forum(int _fid)
{
	sql::ResultSet * res;
	forumGetStatement->setInt(1, _fid);
	res = forumGetStatement->executeQuery();

	if (res->rowsCount() == 0)
	{
		throw badForum;
		return;
	}

	// Loop over the retrieved item..
	fid = _fid;
	res->next();
	name = res->getString("forum_name");
//#ifndef SERVER_CLUSTER
	lastPoster = res->getInt("lastposter");
	lastPost = res->getInt("lastpost");
	lastPosterName = res->getString("lastposter_name");
//#endif
	delete res;
}

Forum::Forum(sql::ResultSet * res)
{
	if (res->rowsCount() == 0)
	{
		throw badForum;
		return;
	}

	// Loop over the retrieved item..
	fid = res->getInt("fid");
	name = res->getString("forum_name");
//#ifndef SERVER_CLUSTER
	lastPoster = res->getInt("lastposter");
	lastPost = res->getInt("lastpost");
	lastPosterName = res->getString("lastposter_name");
//#endif
}

Forum::Forum(sql::ResultSet * res, bool auto_run)
{
	fid = res->getInt("fid");
	name = res->getString("forum_name");
	lastPoster = res->getInt("lastposter");
	lastPost = res->getInt("lastpost");
	lastPosterName = res->getString("lastposter_name");
}

void Forum::prepare()
{
	log("Preparing the forum prepared statements..");
	try
	{
		forumGetTopics = db->con->prepareStatement("SELECT * FROM " + db->prefix + "topics WHERE fid = ? LIMIT 25");
		
		forumGetStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "forums WHERE fid = ? LIMIT 1");
		//forumUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET ? = ? WHERE fid = ? LIMIT 1");
		forumNameUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET forum_name = ? WHERE fid = ? LIMIT 1");
		forumIsStaffUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET is_staff = ? WHERE fid = ? LIMIT 1");
		forumIsAdminUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET is_admin = ? WHERE fid = ? LIMIT 1");
		forumLastPostUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET lastpost = ? WHERE fid = ? LIMIT 1");
		forumLastPosterUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET lastposter = ? WHERE fid = ? LIMIT 1");
		forumLastPosterNameUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET lastposter_name = ? WHERE fid = ? LIMIT 1");
	}
	catch (std::exception& e)
	{
		error(e.what());
	}
}

bool Forum::setName(std::string _name)
{
	name = _name;

	try
	{
		//forumUpdateStatement->setString(1, "name");
		forumNameUpdateStatement->setString(1, _name);
		forumNameUpdateStatement->setInt(2, fid);
		forumNameUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

std::string Forum::getName()
{
	return name;
}

int Forum::getID()
{
	return fid;
}

bool Forum::getStaffOnly()
{
	return staffOnly;
}

bool Forum::getAdminOnly()
{
	return adminOnly;
}

bool Forum::setStaffOnly(bool state)
{
	staffOnly = state;

	try
	{
		forumIsStaffUpdateStatement->setBoolean(1, state);
		forumIsStaffUpdateStatement->setInt(2, fid);
		forumIsStaffUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

bool Forum::setAdminOnly(bool state)
{
	adminOnly = state;

	try
	{
		forumIsAdminUpdateStatement->setBoolean(1, state);
		forumIsAdminUpdateStatement->setInt(2, fid);
		forumIsAdminUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

//#ifndef SERVER_CLUSTER
int Forum::getLastPoster()
{
	return lastPoster;
}

bool Forum::setLastPoster(int _lastPoster)
{
	lastPoster = _lastPoster;

	try
	{
		//forumUpdateStatement->setString(1, "lastposter");
		forumLastPosterUpdateStatement->setInt(1, _lastPoster);
		forumLastPosterUpdateStatement->setInt(2, fid);
		forumLastPosterUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

bool Forum::setLastPoster(int _lastPoster, std::string _lastPosterName)
{
	lastPosterName = _lastPosterName;
	return setLastPoster(_lastPoster);
}

int Forum::getLastPost()
{
	return lastPost;
}

bool Forum::setLastPost(int _lastPost)
{
	lastPost = _lastPost;

	try
	{
		//forumUpdateStatement->setString(1, "lastpost");
		forumLastPostUpdateStatement->setInt(1, _lastPost);
		forumLastPostUpdateStatement->setInt(2, fid);
		forumLastPostUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

bool Forum::setLastPost(Post _lastPost)
{
	return setLastPost(_lastPost.getID());
}
//#endif

sql::ResultSet * Forum::getTopics()
{
	forumGetTopics->setInt(1, fid);
	return forumGetTopics->executeQuery();
}

std::map<int, Forum> getAllForums()
{
	std::map<int, Forum> out;

	sql::Statement * stmt;
	sql::ResultSet * res;
	stmt = db->con->createStatement();
	res = stmt->executeQuery("SELECT * FROM " + db->prefix + "forums");

	log("Iterating over forums..");
	// Loop over the retrieved forums..
	while (res->next())
	{
		out[res->getInt("fid")] = Forum(res);
	}

	delete res;
	delete stmt;
	return out;
}