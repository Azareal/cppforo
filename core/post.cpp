#include "stdafx.h"
#include "post.h"

sql::PreparedStatement * postGetStatement;
sql::PreparedStatement * postUpdateStatement;
ebadField badField;
ebadPost badPost;

void Post::prepare()
{
	try
	{
		postGetStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "posts WHERE pid = ? LIMIT 1");
		postUpdateStatement = db->con->prepareStatement("UPDATE " + db->prefix + "posts SET ? = ? WHERE pid = ? LIMIT 1");
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
	while (res->next())
	{
		content = res->getString("content");
		author = res->getInt("author");
		is_locked = res->getBoolean("is_locked");
		is_sticky = res->getBoolean("is_sticky");
	}

	delete res;
}

int Post::getID()
{
	return pid;
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
		postUpdateStatement->setString(1, "content");
		postUpdateStatement->setString(2, _content);
		postUpdateStatement->setInt(3, pid);
		postUpdateStatement->execute();
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
		postUpdateStatement->setString(1, "author");
		postUpdateStatement->setInt(2, _author);
		postUpdateStatement->setInt(3, pid);
		postUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

bool Post::is(std::string name)
{
	if (name.compare("is_locked")) return is_locked;
	if (name.compare("is_sticky")) return is_sticky;
	throw badField;
	return false;
}

bool Post::lock(bool state)
{
	is_locked = state;

	try
	{
		postUpdateStatement->setString(1, "is_locked");
		postUpdateStatement->setBoolean(2, state);
		postUpdateStatement->setInt(3, pid);
		postUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}

bool Post::stick(bool state)
{
	is_sticky = state;

	try
	{
		postUpdateStatement->setString(1, "is_sticky");
		postUpdateStatement->setBoolean(2, state);
		postUpdateStatement->setInt(3, pid);
		postUpdateStatement->execute();
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}
	return true;
}