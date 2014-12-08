#include "stdafx.h"
#include "forum.h"

// Unfortunately, we can't do ? = ? in prepared statements, so we have loads of them here..
sql::PreparedStatement * forumGetStatement;
//sql::PreparedStatement * forumUpdateStatement;
sql::PreparedStatement * forumNameUpdateStatement;
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
	while (res->next())
	{
		name = res->getString("name");
#ifndef SERVER_CLUSTER
		lastPoster = res->getInt("lastposter");
		lastPost = res->getInt("lastpost");
		lastPosterName = res->getString("lastposter_name");
#endif
	}

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
	name = res->getString("name");
#ifndef SERVER_CLUSTER
	lastPoster = res->getInt("lastposter");
	lastPost = res->getInt("lastpost");
	lastPosterName = res->getString("lastposter_name");
#endif
}

void Forum::prepare()
{
	log("Preparing the forum prepared statements..");
	try
	{
		forumGetStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "forums WHERE fid = ? LIMIT 1");
		//forumUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET ? = ? WHERE fid = ? LIMIT 1");
		forumNameUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "forums SET name = ? WHERE fid = ? LIMIT 1");
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
		forumNameUpdateStatement->setString(2, _name);
		forumNameUpdateStatement->setInt(3, fid);
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

#ifndef SERVER_CLUSTER
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
		forumLastPosterUpdateStatement->setInt(2, _lastPoster);
		forumLastPosterUpdateStatement->setInt(3, fid);
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
	return setLastPoster(_lastPoster);
	lastPosterName = _lastPosterName;
}

bool Forum::setLastPost(int _lastPost)
{
	lastPost = _lastPost;

	try
	{
		//forumUpdateStatement->setString(1, "lastpost");
		forumLastPostUpdateStatement->setInt(2, _lastPost);
		forumLastPostUpdateStatement->setInt(3, fid);
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
#endif

std::map<int, Forum> getAllForums()
{
	log("Entering getAllForums");
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
	log("Return from getAllForums");
	return out;
}