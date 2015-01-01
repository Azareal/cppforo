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
RouteMap routes;
std::map<int, Forum> forums;

// The main configuration file variable..
boost::property_tree::ptree pt;

void forum_server::handle_request(http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn, const boost::system::error_code& ec)
{
	std::string path = http_request_ptr->get_resource();
	boost::asio::ip::address ip = tcp_conn->get_remote_ip();
	log("Received new request from '" + ip.to_string() + "' who is trying to access '" + path + "'");
	std::string route;

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
	}
	// Home is the default route..
	else routes["home"]("", http_request_ptr, tcp_conn);
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
sql::PreparedStatement * getSessionStmt;
sql::PreparedStatement * getTopicStmt;
sql::PreparedStatement * createReplyStmt;
sql::PreparedStatement * createTopicStmt;
sql::PreparedStatement * getInsertId;

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
