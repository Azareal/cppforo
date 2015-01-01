#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include "templates.h"
#include "forum.h"
#include "topic.h"
#include "user.h"
//#include "parse.h"
#include "routes.h"

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <boost/asio.hpp>
#include <pion/config.hpp>
#include <pion/error.hpp>
#include <pion/plugin.hpp>
#include <pion/process.hpp>
#include <pion/tcp/server.hpp>
#include <pion/tcp/connection.hpp>
#include <pion/http/server.hpp>
#include <pion/http/request.hpp>
#include <pion/http/request_reader.hpp>
#include <pion/http/response_writer.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace pion;

class forum_server : public http::server
{
public:
	explicit forum_server(const unsigned int tcp_port = 0) : http::server(tcp_port)
	{
		set_logger(PION_GET_LOGGER("cppforo.forum_server"));
	}

	virtual void handle_request(http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn, const boost::system::error_code& ec);
};