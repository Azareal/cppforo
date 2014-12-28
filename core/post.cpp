/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "post.h"

sql::PreparedStatement * postGetStatement;
sql::PreparedStatement * postGetParentTopicStatement;
sql::PreparedStatement * postContentUpdateStatement;
sql::PreparedStatement * postTIDUpdateStatement;
sql::PreparedStatement * postAuthorUpdateStatement;
ebadPost badPost;

void Post::prepare()
{
	try
	{
		postGetStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "posts WHERE pid = ? LIMIT 1");
		//postUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "posts SET ? = ? WHERE pid = ? LIMIT 1");
		postContentUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "posts SET content = ? WHERE pid = ? LIMIT 1");
		postTIDUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "posts SET tid = ? WHERE pid = ? LIMIT 1");
		postAuthorUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "posts SET author = ? WHERE pid = ? LIMIT 1");
		postGetParentTopicStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "topics WHERE tid = ? LIMIT 1");
	}
	catch (std::exception& e)
	{
		error(e.what());
	}
}

Post::Post(int _pid)
{
	sql::ResultSet * res;
	postGetStatement->setInt(1, _pid);
	res = postGetStatement->executeQuery();

	if (res->rowsCount() == 0)
	{
		throw badPost;
		return;
	}

	// Loop over the retrieved item..
	pid = _pid;
	res->next();
	content = res->getString("content");
	author = res->getInt("author");
	tid = res->getInt("tid");

	delete res;
}

int Post::getID()
{
	return pid;
}

int Post::getTID()
{
	return tid;
}

bool Post::setTID(int _tid)
{
	tid = _tid;

	try
	{
		//postUpdateStatement->setString(1, "content");
		postTIDUpdateStatement->setInt(1, _tid);
		postTIDUpdateStatement->setInt(2, pid);
		postTIDUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

std::string Post::getContent()
{
	return content;
}

bool Post::setContent(std::string _content)
{
	content = _content;

	try
	{
		postContentUpdateStatement->setString(1, _content);
		postContentUpdateStatement->setInt(2, pid);
		postContentUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

int Post::getAuthor()
{
	return author;
}

bool Post::setAuthor(int _author)
{
	author = _author;

	try
	{
		postAuthorUpdateStatement->setInt(1, _author);
		postAuthorUpdateStatement->setInt(2, pid);
		postAuthorUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

ebadTopic badTopic;
Topic Post::getParentTopic()
{
	log(std::to_string(tid));
	Topic parentTopic(tid);
	return parentTopic;
}