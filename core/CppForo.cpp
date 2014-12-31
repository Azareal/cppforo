/*
	CppForo Main Entrypoint
	Copyright (c) Azareal 2014.
	Licensed under the LGPLv3.
*/

#include "stdafx.h"
#include "Cppforo.h"

// The database globals..
sql::mysql::MySQL_Driver * driver;
sql::Connection * con;
ForoDatabase * db;
Templates * tmpls;

// The settings..
std::map<std::string, std::string> settings;
//typedef std::function<std::string(std::string, dlib::incoming_things, dlib::outgoing_things&)> Route;
typedef std::function<void(std::string, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)> Route;
typedef std::map<std::string, Route> RouteMap;
RouteMap routes;
std::map<int, Forum> forums;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>

// The main configuration file variable..
boost::property_tree::ptree pt;

void forum_server::handle_request(http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn, const boost::system::error_code& ec)
//const std::string on_request(const dlib::incoming_things& incoming, dlib::outgoing_things& outgoing)
{
	std::string path = http_request_ptr->get_resource();
	boost::asio::ip::address ip = tcp_conn->get_remote_ip();
	//std::string path = incoming.path;
	log("Received new request from '" + ip.to_string() + "' who is trying to access '" + path + "'");
	std::string route;
	//std::string out;

	// Is this conditional really needed..?
	if (path.size() > 1)
	{
		std::string::iterator it;
		for (it = path.begin() + 1; it != path.end(); ++it)
		{
			if (*it == '/') break;
		}
		
		if (it != path.end())
		{
			try
			{
				int index = std::distance(path.begin() + 1, it);
				//out << routes[path.substr(1, index)](path.substr(index + 2), incoming, outgoing);
				//return routes[path.substr(1, index)](path.substr(index + 2), incoming, outgoing);
				routes[path.substr(1, index)](path.substr(index + 2), http_request_ptr, tcp_conn);
			}
			catch (std::exception& e)
			{
				ferror(e.what());
				http::response_writer_ptr writer(
					http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));

				http::response res = writer->get_response();
				res.set_content_type(http::types::CONTENT_TYPE_HTML);
				res.set_status_code(500);
				res.set_status_message("Internal Server Error");

				// Push the custom error page..
				writer->write("Invalid route");
				writer->send();
			}
		}
		else routes["home"](path.substr(1), http_request_ptr, tcp_conn);
		//else return routes["home"](path.substr(1), incoming, outgoing);
	}
	// Home is the default route..
	else routes["home"]("", http_request_ptr, tcp_conn);
	//else return routes["home"]("",incoming, outgoing);
}

void addRoute(std::string name, Route route)
{
	routes[name] = route;
}

void loadSettings()
{
	sql::Statement * stmt;
	sql::ResultSet * res;
	stmt = con->createStatement();
	res = stmt->executeQuery("SELECT * FROM " + pt.get<std::string>("mysql.prefix") + "settings");

	// Loop over the retrieved settings..
	while(res->next())
	{
		settings[res->getString("name")] = res->getString("content");
	}

	delete res;
	delete stmt;
}

// The prepared statements..
// TO-DO: Stop storing these as globals..
sql::PreparedStatement * getSessionStmt;
sql::PreparedStatement * getTopicStmt;
sql::PreparedStatement * createReplyStmt;
sql::PreparedStatement * createTopicStmt;
sql::PreparedStatement * getInsertId;

//User checkSession(dlib::incoming_things& incoming)
User checkSession(pion::http::request_ptr& http_request_ptr)
{
	int uid = 0;
	std::string session = "";

	// TO-DO: Use a lexical cast for this..
	try{
		//uid = std::stoi(incoming.cookies["uid"]);
		//session = incoming.cookies["session"];
		uid = std::stoi(http_request_ptr->get_cookie("uid"));
		session = http_request_ptr->get_cookie("session");
	}
	catch (...)
	{
		//log("It's a guest!");
		return guest();
	}

	if (uid != 0 && session.compare("")!=0)
	{
		//log("The session isn't blank");

		sql::ResultSet * res;
		getSessionStmt->setString(1, session);
		res = getSessionStmt->executeQuery();

		if (res->rowsCount() == 0)
		{
			// TO-DO: Might have to add a more detailed logging level to avoid the admin console from being flooded..
			//log("It's a guest!");
			return guest();
		}

		User out;
		out.uid = res->getInt("uid");
		out.username = res->getString("username");
		out.gid = res->getInt("gid");
		out.is_admin = res->getBoolean("is_admin");
		out.is_mod = res->getBoolean("is_mod");
		out.is_banned = res->getBoolean("is_banned");

		delete res;
		return out;
	}

	return guest();
}

void membersonly_error(http::response_writer_ptr& writer)
{
	log("Non-members aren't allowed on a members only site.");
	pion::http::response res = writer->get_response();
	res.set_status_code(403);
	res.set_status_message("Forbidden");

	// Push the custom error page..
	writer->write("You aren't allowed in this area!");
	writer->send();
	return;
}

void banned_error(http::response_writer_ptr& writer)
{
	pion::http::response res = writer->get_response();
	res.set_status_code(403);
	res.set_status_message("Forbidden");

	// Push the banned page..
	writer->write(tmpls->render("banned"));
	writer->send();
	return;
}

void init_default_routes()
{
	addRoute("test", [](std::string path, pion::http::request_ptr& http_request_ptr, pion::tcp::connection_ptr& tcp_conn)->void{
		log("Entering the test route..");
		pion::http::response_writer_ptr writer(
			pion::http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&pion::tcp::connection::finish, tcp_conn)));
		writer->get_response().set_content_type(pion::http::types::CONTENT_TYPE_HTML);
		writer->write("Hello World");
		writer->send();
	});
	//addRoute("home", [](std::string path, dlib::incoming_things incoming, dlib::outgoing_things& outgoing)->std::string{
	addRoute("home", [](std::string path, pion::http::request_ptr& http_request_ptr, pion::tcp::connection_ptr& tcp_conn)->void{
		log("Entering the home route..");
		// TO-DO: Perhaps, we should use a using statement to avoid having to type "pion::" so much..
		pion::http::response_writer_ptr writer(
			pion::http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&pion::tcp::connection::finish, tcp_conn)));

		//User currentUser = checkSession(incoming);
		User currentUser = checkSession(http_request_ptr);

		// Is the browser looking for the favicon..?
		if (path.compare("favicon.ico") == 0)
		{
			pion::http::response res = writer->get_response();
			res.set_content_type("image/x-icon");
			res.set_status_code(404);
			res.set_status_message("File Not Found");

			// A custom error page might not be appropriate for an icon..
			//writer->write("");
			writer->send();
			//outgoing.http_return = 404;
			//outgoing.http_return_status = "File Not Found";
			return;
		}

		// This is probably a HTML page then..
		writer->get_response().set_content_type(pion::http::types::CONTENT_TYPE_HTML);

		//tmpls->assignVar("helloworld", "Hello World");
		if (!currentUser.loggedIn() && settings["members_only"].compare("1") == 0) return membersonly_error(writer);
		if (currentUser.is_banned) return banned_error(writer);

		std::string forumList;
		User lastposter;
		for (auto it = forums.begin(); it != forums.end(); ++it)
		{
			if (it->second.getAdminOnly() && !currentUser.is_admin) continue;
			if (it->second.getStaffOnly() && !currentUser.is_mod) continue;

			tmpls->assignVar("fid", std::to_string(it->second.getID()));
			tmpls->assignVar("forum_name", it->second.getName());

			// If the lastposter doesn't exist then, the account may have been deleted.
			try
			{
				User lastposter(it->second.getLastPoster());
			}
			catch (...) { lastposter = guest(); }
			tmpls->assignVar("lastposter_name", lastposter.getName());

			Post lastpost(it->second.getLastPost());
			// TO-DO: Use a join here for extra performance instead of two queries
			Topic lastTopic = lastpost.getParentTopic();
			tmpls->assignVar("lastpost_name", lastTopic.getName());
			tmpls->assignVar("lastpost_tid", std::to_string(lastTopic.getID()));

			forumList += tmpls->render("forum_row");
		}

		tmpls->assignVar("forum_list", forumList);
		tmpls->assignVar("catname", "Forum List");
		tmpls->assignVar("body", tmpls->render("category"));

		log("Pushing out the page..");
		tmpls->assignVar("header", tmpls->render("header"));
		tmpls->assignVar("footer", tmpls->render("footer"));

		//log(tmpls->render("page"));
		writer->write(tmpls->render("page"));
		writer->send();
		//return tmpls->render("page");
	});
	//addRoute("topics", [](std::string path, dlib::incoming_things incoming, dlib::outgoing_things& outgoing)->std::string{
	addRoute("topics", [](std::string path, pion::http::request_ptr& http_request_ptr, pion::tcp::connection_ptr& tcp_conn)->void{
		//log("Entering topic '" + path + "'");
		pion::http::response_writer_ptr writer(
			pion::http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&pion::tcp::connection::finish, tcp_conn)));
		writer->get_response().set_content_type(pion::http::types::CONTENT_TYPE_HTML);

		User currentUser = checkSession(http_request_ptr);

		if (!currentUser.loggedIn() && settings["members_only"].compare("1") == 0) return membersonly_error(writer);
		if (currentUser.is_banned) return banned_error(writer);

		int tid = 0;
		try { tid = boost::lexical_cast<int>(path); }
		catch (const boost::bad_lexical_cast &) { tid = 0; }

		sql::ResultSet * res;
		getTopicStmt->setInt(1, tid);
		res = getTopicStmt->executeQuery();
		//log("Fetching the topic..");
		if (res->rowsCount() == 0)
		{
			log("The topic doesn't exist!");
			pion::http::response res = writer->get_response();
			res.set_content_type(pion::http::types::CONTENT_TYPE_HTML);
			res.set_status_code(404);
			res.set_status_message("File Not Found");

			// Push the custom error page..
			writer->write("The topic doesn't exist!");
			writer->send();
			return;
		}

		// Get the table row..
		res->next();
		Topic mainTopic(res);
		tmpls->assignVar("tid", std::to_string(mainTopic.getID()));
		tmpls->assignVar("topic_name", mainTopic.getName());
		Forum parentForum(res, false);
		delete res;

		std::string postList;
		res = mainTopic.getPosts();
		while (res->next())
		{
			tmpls->assignVar("content", res->getString("content"));

			User postAuthor(res->getInt("author"));
			tmpls->assignVar("username", postAuthor.getName());

			postList += tmpls->render("post");
		}
		delete res;
		tmpls->assignVar("postList", postList);
		tmpls->assignVar("body", tmpls->render("topic"));

		log("Pushing the page..");
		tmpls->assignVar("header", tmpls->render("header"));
		tmpls->assignVar("footer", tmpls->render("footer"));

		writer->write(tmpls->render("page"));
		writer->send();
		//return tmpls->render("page");
	});
	//addRoute("forums", [](std::string path, dlib::incoming_things incoming, dlib::outgoing_things& outgoing)->std::string{
	addRoute("forums", [](std::string path, pion::http::request_ptr& http_request_ptr, pion::tcp::connection_ptr& tcp_conn)->void{
		log("Entering forum '" + path + "'");
		pion::http::response_writer_ptr writer(
			pion::http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&pion::tcp::connection::finish, tcp_conn)));

		User currentUser = checkSession(http_request_ptr);
		if (!currentUser.loggedIn() && settings["members_only"].compare("1") == 0) return membersonly_error(writer);
		if (currentUser.is_banned) return banned_error(writer);

		int fid = 0;
		try { fid = boost::lexical_cast<int>(path); }
		catch (const boost::bad_lexical_cast &) { fid = 0; }

		// Luckily, we can just fetch the forum data from memory instead of having to waste a query..
		log("Fetching the forum..");
		Forum forum;
		try { forum = forums[fid]; }
		catch (...)
		{
			log("The forum doesn't exist!");
			pion::http::response res = writer->get_response();
			res.set_content_type(pion::http::types::CONTENT_TYPE_HTML);
			res.set_status_code(404);
			res.set_status_message("File Not Found");

			// Push the custom error page..
			writer->write("This forum doesn't exist!");
			writer->send();
		}

		// Assign the forum data to the templates..
		// TO-DO: Map the forum data to the templates when the forum server is initialized..
		tmpls->assignVar("fid", std::to_string(forum.getID()));
		tmpls->assignVar("forum_name", forum.getName());

		std::string topicList;
		sql::ResultSet * res;
		res = forum.getTopics();
		while (res->next())
		{
			tmpls->assignVar("tid", std::to_string(res->getInt("tid")));
			tmpls->assignVar("topic_name", res->getString("topic_name"));

			User postAuthor(res->getInt("author"));
			tmpls->assignVar("author_name", postAuthor.getName());
			// TO-DO: Store the lastposter data on a per-topic level..
			tmpls->assignVar("lastposter_name", postAuthor.getName());

			topicList += tmpls->render("topic_row");
		}
		delete res;
		tmpls->assignVar("topic_list", topicList);
		tmpls->assignVar("body", tmpls->render("forum"));

		log("Pushing the page..");
		tmpls->assignVar("header", tmpls->render("header"));
		tmpls->assignVar("footer", tmpls->render("footer"));

		writer->write(tmpls->render("page"));
		writer->send();
	});
	addRoute("create", [](std::string path, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)->void{
		http::response_writer_ptr writer(
			http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));
		
		User currentUser = checkSession(http_request_ptr);
		// TO-DO: Add the ability to restrict posting to members only.
		/*if (!currentUser.loggedIn() && settings["members_only"].compare("1") == 0)
		{
			return ""; // TO-DO: Return a 403.
		}*/
		if (currentUser.is_banned) return banned_error(writer);
		
		pion::http::response resp = writer->get_response();
		resp.set_content_type(pion::http::types::CONTENT_TYPE_HTML);
		std::string body = http_request_ptr->get_content();
		log(body);

		if (path.compare("topic") == 0)
		{
			int fid = 0;
			std::string name;
			std::string content;
			
			// TO-DO: Use a less restrictive filtering method, since this one doesn't accept ANY symbols :(
			//log("Parsing the incoming form data..");
			std::map<std::string, std::string> items = parseQueryString(body);
			
			// Convert this to an integer.. Keep in mind, that this might not be a real integer, so set to 0 when the end-user is being stupid..
			log("Converting the FID..");
			try { fid = boost::lexical_cast<int>(items["fid"]); }
			catch (const boost::bad_lexical_cast &) { fid = 0; }
			
			try { name = items["name"]; }
			catch (...) { log("No title!"); return; }

			try { content = items["content"]; }
			catch (...) { log("No content!"); return; }

			// We want to execute these queries in a single transaction..
			sql::ResultSet * res;
			db->con->setAutoCommit(0);

			log("Firing off the insert queries!");
			int tid;
			try
			{
				createTopicStmt->setString(1, name);
				createTopicStmt->setInt(2, 0); // Author
				createTopicStmt->setInt(3, fid);
				//res = createTopicStmt->executeQuery();
				createTopicStmt->execute();

				res = getInsertId->executeQuery();
				res->next();
				tid = res->getInt("last_id");
				delete res;

				createReplyStmt->setString(1, content);
				createReplyStmt->setInt(2, 0); // Author
				createReplyStmt->setInt(3, tid);
				createReplyStmt->execute();
			}
			catch (std::exception& e)
			{
				log(e.what());
				db->con->rollback();
				db->con->setAutoCommit(1);
				resp.set_status_code(500);
				resp.set_status_message("Internal Server Error");
				writer->send();
				return;
			}

			// Release the database lock and switch auto-commit back on..
			db->con->commit();
			db->con->setAutoCommit(1);

			resp.set_status_code(302);
			resp.set_status_message("Found");
			resp.add_header("Location", settings["site_url"] + "/topics/" + std::to_string(tid));
			writer->send();
			return;
		}
		else if (path.compare("post") == 0)
		{
			int tid = 0;
			std::string content;
				
			// TO-DO: Use a less restrictive filtering method, since this one doesn't accept ANY symbols..
			//log("Parsing the incoming form data..");
			std::map<std::string,std::string> items = parseQueryString(body);
				
			// Convert this to an integer.. Keep in mind, that this might not be a real integer, so set to 0 when the end-user is being stupid..
			//log("Converting the TID..");
			try { tid = boost::lexical_cast<int>(items["tid"]); }
			catch (const boost::bad_lexical_cast &) { tid = 0; }
			
			//log("Passing the content..");
			try { content = items["content"]; }
			catch (...) { log("No content!"); return; }
			
			
			//log("Firing off the insert query!");
			try
			{
				createReplyStmt->setString(1, content);
				createReplyStmt->setInt(2, 0); // Author
				createReplyStmt->setInt(3, tid);
				createReplyStmt->execute();
			}
			catch (std::exception& e)
			{
				log(e.what());
				resp.set_status_code(500);
				resp.set_status_message("Internal Server Error");
				writer->send();
				return;
			}
			
			resp.set_status_code(302);
			resp.set_status_message("Found");
			resp.add_header("Location", settings["site_url"] + "/topics/" + std::to_string(tid));
			writer->send();
			return;
		}
		
		resp.set_status_code(404);
		resp.set_status_message("File Not Found");

		// Push the custom error page..
		writer->write("This creation action doesn't exist!");
		writer->send();
	});
	addRoute("login", [](std::string path, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)->void{
		http::response_writer_ptr writer(
			http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));

		User currentUser = checkSession(http_request_ptr);
		if (currentUser.loggedIn())
		{
			log("You can't login when you're already logged in..");
			pion::http::response res = writer->get_response();
			res.set_content_type(pion::http::types::CONTENT_TYPE_HTML);
			res.set_status_code(403);
			res.set_status_message("Forbidden");

			// Push the custom error page..
			writer->write("You can't login when you're already logged in..");
			writer->send();
			return;
		}
		if (currentUser.is_banned) return banned_error(writer);

		// TO-DO: The Login System.

		log("Pushing the page..");
		tmpls->assignVar("header", tmpls->render("header"));
		tmpls->assignVar("footer", tmpls->render("footer"));

		writer->write(tmpls->render("page"));
		writer->send();
	});

	/* The Static Routes */
	// TO-DO: Add an add_static_resource_route function somehow.
	// TO-DO: Add a setting for pre-loading some of these static resources into memory instead of manually reading the files every time..
	//addRoute("js", [](std::string path, dlib::incoming_things incoming, dlib::outgoing_things& outgoing)->std::string{
	addRoute("js", [](std::string path, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)->void{
		http::response_writer_ptr writer(
			http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));

		// Try to avoid dangerous characters being introduced with minimal performance issues..
		path = sanitisePathStrict(path);
		//if (!validatePathStrict(path)) return "";  // TO-DO: Return a 500 or whatever the bad request code was.

		std::string data;
		std::ifstream in("../js/" + path + ".js", std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			data.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&data[0], data.size());
			in.close();

			writer->write(data);
			writer->send();
			return;
		}

		http::response res = writer->get_response();
		res.set_status_code(404);
		res.set_status_message("File Not Found");
		writer->send();
	});
	addRoute("css", [](std::string path, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)->void{
		http::response_writer_ptr writer(
			http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));

		// Try to avoid dangerous characters being introduced with minimal performance issues..
		path = sanitisePathStrict(path);
		//if (!validatePathStrict(path)) return "";  // TO-DO: Return a 500 or whatever the bad request code was.

		writer->get_response().set_content_type("text/css");
		//outgoing.headers["Content-Type"] = "text/css";
		std::ifstream in("../css/" + path + ".css", std::ios::in | std::ios::binary);
		if (in)
		{
			//log("The CSS file exists!");
			std::string data;
			in.seekg(0, std::ios::end);
			data.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&data[0], data.size());
			in.close();

			writer->write(data);
			writer->send();
			return;
		}

		http::response res = writer->get_response();
		res.set_status_code(404);
		res.set_status_message("File Not Found");
		writer->send();
	});
	addRoute("images", [](std::string path, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)->void{
		http::response_writer_ptr writer(
			http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn)));

		//log("Pushing an image file..");
		path = sanitisePathLoose(path);
		log(path);

		//writer->get_response().set_content_type("text/css");
		//log("Valid image path!");
		std::ifstream in("../images/" + path, std::ios::in | std::ios::binary);
		if (in)
		{
			//log("The image file exists!");
			std::string data;
			in.seekg(0, std::ios::end);
			data.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&data[0], data.size());
			in.close();

			writer->write(data);
			writer->send();
			return;
		}

		http::response res = writer->get_response();
		res.set_status_code(404);
		res.set_status_message("File Not Found");
		writer->send();
	});
}

// The main entrypoint..
int main(int argc, char * argv[])
{
	// TO-DO: We need to make use of Python..
	//log("Initializing the Python interpreter..");
	//Py_Initialize();

	log("Reading the configuration file..");
	try
	{
		boost::property_tree::ini_parser::read_ini("../config.ini", pt);
	}
	catch (std::exception& e)
	{
		std::string errmsg(e.what());
		ferror("Exception:" + errmsg);
		return 1;
	}

	std::string address;
	std::string port;
	if (argc != 3) {
		//std::cerr << "Usage: " << argv[0] << " address port" << std::endl;
		try { address = pt.get<std::string>("server.address").c_str(); }
		catch (...) { address = "localhost"; }
		try { port = pt.get<std::string>("server.port").c_str(); }
		catch (...) { port = "2555"; }
	}
	else
	{
		address = argv[1];
		port = argv[2];
	}

	std::string mysql_address;
	std::string mysql_port;
	try {mysql_address = pt.get<std::string>("mysql.address");}
	catch (...) {mysql_address = "localhost"; }
	try {mysql_port = pt.get<std::string>("mysql.port");}
	catch (...) { mysql_port = "3306"; }

	log("Initializing MySQL..");
	driver = sql::mysql::get_mysql_driver_instance();

	log("Connecting to MySQL..");
	try
	{
		con = driver->connect("tcp://" + mysql_address + ":" + mysql_port,
			pt.get<std::string>("mysql.username"),
			pt.get<std::string>("mysql.password")
		);
	}
	catch (std::exception& e)
	{
		std::string errmsg(e.what());
		ferror("Exception:" + errmsg);
		return 1;
	}

	log("Switching to the correct database..");
	sql::Statement * stmt;
	stmt = con->createStatement();
	try
	{
		stmt->execute("USE " + pt.get<std::string>("mysql.database"));
	}
	catch (std::exception& e)
	{
		std::string errmsg = e.what();
		ferror("Exception: " + errmsg);
		return 1;
	}
	delete stmt;

	// Load the "non-core" settings..
	log("Loading the settings..");
	try { loadSettings(); }
	catch (std::exception& e)
	{
		std::string errmsg = e.what();
		log("Failed to load the settings..");
		ferror("Exception: " + errmsg);
		return 1;
	}
	
	log("Moving the database handling into the database object..");
	db = new ForoDatabase(con, pt.get<std::string>("mysql.prefix"));

	log("Prepare the prepared statements..");
	Forum::prepare();
	User::prepare();
	Topic::prepare();
	Post::prepare();

	try
	{
		getSessionStmt = db->con->prepareStatement("SELECT * FROM " + db->prefix + "users WHERE session = ? LIMIT 1");
		getTopicStmt = db->con->prepareStatement("SELECT * FROM " + db->prefix + "topics AS topics LEFT JOIN " + db->prefix + "forums AS forums ON topics.fid=forums.fid WHERE tid = ? LIMIT 1");
		createReplyStmt = db->con->prepareStatement("INSERT INTO " + db->prefix + "posts (content, author, tid) VALUES (?, ?, ?)");
		createTopicStmt = db->con->prepareStatement("INSERT INTO " + db->prefix + "topics (topic_name, author, fid) VALUES (?, ?, ?)");
		
		// Unfortunately, the C++ API doesn't see to cover this for me.. sooooooo....
		getInsertId = db->con->prepareStatement("SELECT LAST_INSERT_ID() AS last_id;");
	}
	catch (std::exception& e)
	{
		std::string errmsg = e.what();
		ferror("Exception: " + errmsg);
		return 1;
	}

	log("Loading the forums..");
	forums = getAllForums();

	log("Loading the templates..");
	tmpls = new Templates(db);
	if (!tmpls->loadTemplate("header")) { ferror("Failed to load the header template.."); return 1; }
	if (!tmpls->loadTemplate("footer")) { ferror("Failed to load the footer template.."); return 1; }
	if (!tmpls->loadTemplate("page")) { ferror("Failed to load the page template.."); return 1; }

	if (!tmpls->loadTemplate("category")) { ferror("Failed to load the category template.."); return 1; }
	if (!tmpls->loadTemplate("forum_row")) { ferror("Failed to load the forum_row template.."); return 1; }

	if (!tmpls->loadTemplate("forum")) { ferror("Failed to load the forum template.."); return 1; }
	if (!tmpls->loadTemplate("topic_row")) { ferror("Failed to load the topic_row template.."); return 1; }

	if (!tmpls->loadTemplate("topic")) { ferror("Failed to load the topic template.."); return 1; }
	if (!tmpls->loadTemplate("post")) { ferror("Failed to load the post template.."); return 1; }

	log("Caching some of the very frequently used settings..");
	tmpls->assignVar("site_url", settings["site_url"]);
	tmpls->assignVar("site_name", settings["site_name"]);

	log("Registering the routes..");
	init_default_routes();

	log("Initializing the forum server..");
	//forum_server fserver;
	forum_server fserver(atoi(port.c_str()));
	try
	{
		//process::initialize();
		
		//forum_server  fserver = forum_server::server(atoi(port.c_str()));
		//auto fserver = forum_server::server(atoi(port.c_str()));
		//fserver.set_port(atoi(port.c_str()));
		fserver.start();
		//process::wait_for_shutdown();
	}
	catch (std::exception& e)
	{
		std::string errmsg = e.what();
		ferror("Exception: " + errmsg);
		return 1;
	}

	//forum_server fserver;
	//fserver.set_listening_port(atoi(port.c_str()));
	//fserver.start_async();

	/*bool shutdown = false;
	while (!shutdown)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}*/

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);

	log("Shutting down the main forum server..");
	fserver.stop();

	//log("Cleaning up Python..");
	//Py_Finalize();

	log("Cleaning up MySQL..");
	delete con;

	return 0;
}
