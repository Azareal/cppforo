// CppForo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include "templates.h"
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
Database * db;
Templates * tmpls;

// The settings..
std::map<std::string, std::string> settings;

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

		out << "Hello World";

		return out.str();
	}
};

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

	log("Loading the templates..");
	db = new Database(con, pt.get<std::string>("mysql.prefix"));
	tmpls = new Templates(db);
	if (!tmpls->loadTemplate("header")) { error("Failed to load the header template.."); return 1; }
	if (!tmpls->loadTemplate("footer")) { error("Failed to load the footer template.."); return 1; }
	if (!tmpls->loadTemplate("page")) { error("Failed to load the page template.."); return 1; }

	log("Initializing the forum server..");
	forum_server fserver;
	fserver.set_listening_port(atoi(port.c_str()));
	fserver.start_async();

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
