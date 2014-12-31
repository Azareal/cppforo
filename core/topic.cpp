#include "stdafx.h"
#include "topic.h"

sql::PreparedStatement * topicGetPosts;
sql::PreparedStatement * topicGetStatement;
sql::PreparedStatement * topicNameUpdateStatement;
sql::PreparedStatement * topicAuthorUpdateStatement;
sql::PreparedStatement * topicIsLockedUpdateStatement;
sql::PreparedStatement * topicFidUpdateStatement;

extern ebadTopic badTopic;
Topic::Topic()
{
	tid = 0;
	name = "";
	author = 0;
	is_locked = false;
	fid = 0;
}

Topic::Topic(int _tid)
{
	sql::ResultSet * res;
	topicGetStatement->setInt(1, _tid);
	res = topicGetStatement->executeQuery();

	if (res->rowsCount() == 0)
	{
		throw badTopic;
		return;
	}

	tid = _tid;

	// Get the table row..
	res->next();

	name = res->getString("topic_name");
	author = res->getInt("author");
	is_locked = res->getBoolean("is_locked");
	fid = res->getInt("fid");

	delete res;
}

Topic::Topic(sql::ResultSet * res)
{
	tid = res->getInt("tid");
	name = res->getString("topic_name");
	author = res->getInt("author");
	is_locked = res->getBoolean("is_locked");
	fid = res->getInt("fid");
}

void Topic::prepare()
{
	log("Preparing the forum prepared statements..");
	try
	{
		topicGetPosts = db->con->prepareStatement("SELECT * FROM " + db->prefix + "posts WHERE tid = ? LIMIT 25");
		
		topicGetStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "topics WHERE tid = ? LIMIT 1");
		topicNameUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "topics SET topic_name = ? WHERE tid = ? LIMIT 1");
		topicAuthorUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "topics SET author = ? WHERE tid = ? LIMIT 1");
		topicIsLockedUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "topics SET is_locked = ? WHERE tid = ? LIMIT 1");
		topicFidUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "topics SET fid = ? WHERE tid = ? LIMIT 1");
	}
	catch (std::exception& e)
	{
		ferror(e.what());
	}
}

int Topic::getID()
{
	return tid;
}

int Topic::getFID()
{
	return fid;
}

bool Topic::setFID(int _fid)
{
	fid = _fid;

	try
	{
		topicFidUpdateStatement->setInt(1, _fid);
		topicFidUpdateStatement->setInt(2, tid);
		topicFidUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

std::string Topic::getName()
{
	return name;
}

bool Topic::setName(std::string _name)
{
	name = _name;

	try
	{
		topicNameUpdateStatement->setString(1, _name);
		topicNameUpdateStatement->setInt(2, tid);
		topicNameUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

int Topic::getAuthor()
{
	return author;
}

bool Topic::setAuthor(int _author)
{
	author = _author;

	try
	{
		topicAuthorUpdateStatement->setInt(1, _author);
		topicAuthorUpdateStatement->setInt(2, tid);
		topicAuthorUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;

}

bool Topic::isLocked()
{
	return is_locked;
}

bool Topic::setLocked(bool _locked)
{
	is_locked = _locked;

	try
	{
		topicIsLockedUpdateStatement->setBoolean(1, _locked);
		topicIsLockedUpdateStatement->setInt(2, tid);
		topicIsLockedUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

sql::ResultSet * Topic::getPosts()
{
	topicGetPosts->setInt(1, tid);
	return topicGetPosts->executeQuery();
}