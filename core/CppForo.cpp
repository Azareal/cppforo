// CppForo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <functional>
#include "templates.h"
#include "forum.h"
//#include "server_http_thread_pool.h"

//#include <boost/network/protocol/http/server.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

#define ENABLE_ASSERTS
#include <dlib/all/source.cpp>
#include <dlib/server.h>

// The database globals..
sql::mysql::MySQL_Driver * driver;
sql::Connection * con;
ForoDatabase * db;
Templates * tmpls;

// The settings..
std::map<std::string, std::string> settings;
typedef std::function<std::string(std::string, dlib::incoming_things, dlib::outgoing_things)> Route;
typedef std::map<std::string, Route> RouteMap;
RouteMap routes;
std::map<int, Forum> forums;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// The main configuration file variable..
boost::property_tree::ptree pt;

class forum_server : public dlib::server_http
//class forum_server : public server_http_thread_pool
{
	const std::string on_request(const dlib::incoming_things& incoming, dlib::outgoing_things& outgoing)
	{
		std::ostringstream out;
		std::string path = incoming.path;
		log("Received new request from '" + incoming.foreign_ip + "' who is trying to access '" + path + "'");
		std::string route;

		if (path.size() != 1)
		{
			std::string::iterator it;
			for (it = path.begin() + 1; it != path.end(); ++it)
			{
				if (*it == '/') break;
			}
			int index = std::distance(path.begin(), it);
			try{
				out << routes[path.substr(0, index)](path.substr(index), incoming, outgoing);
			}
			catch (std::exception& e)
			{
				error(e.what());
				return "500 Internal Server Error";
			}
		}
		// Home is the default route..
		else out << routes["home"]("",incoming, outgoing);

		log("Responding to the request!");
		return out.str();
	}
};

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

// The main entrypoint..
int main(int argc, char * argv[])
{
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
		error("Exception:" + errmsg);
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
		error("Exception:" + errmsg);
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
		error("Exception: " + errmsg);
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
		error("Exception: " + errmsg);
		return 1;
	}
	
	log("Moving the database handling into the database object..");
	db = new ForoDatabase(con, pt.get<std::string>("mysql.prefix"));
	Forum::prepare();

	log("Loading the forums..");
	forums = getAllForums();

	log("Loading the templates..");
	tmpls = new Templates(db);
	if (!tmpls->loadTemplate("header")) { error("Failed to load the header template.."); return 1; }
	if (!tmpls->loadTemplate("footer")) { error("Failed to load the footer template.."); return 1; }
	if (!tmpls->loadTemplate("page")) { error("Failed to load the page template.."); return 1; }

	log("Registering the routes..");
	addRoute("home", [](std::string path, dlib::incoming_things& incoming, dlib::outgoing_things& outgoing){
		tmpls->assignVar("helloworld", "Hello World");
		std::string page = tmpls->render("page");
		return page;
	});

	log("Initializing the forum server..");
	forum_server fserver;
	fserver.set_listening_port(atoi(port.c_str()));
	fserver.start_async();

	/*bool shutdown = false;
	while (!shutdown)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}*/

	// Stop the console from exitting immediately..
	log("Press enter to exit..");
	std::string blargle;
	std::getline(std::cin, blargle);

	//log("Cleaning up Python..");
	//Py_Finalize();

	log("Cleaning up MySQL..");
	delete con;

	return 0;
}
