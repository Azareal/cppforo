#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <functional>
#include "templates.h"
#include "forum.h"
#include "topic.h"
#include "user.h"
#include "parse.h"
//#include "server_http_thread_pool.h"

//#include <boost/network/protocol/http/server.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

//#define ENABLE_ASSERTS
//#include <dlib/all/source.cpp>
//#include <dlib/server.h>

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

using namespace pion;

class forum_server : public http::server
{
public:
	//explicit server(const unsigned int tcp_port = 0) : http::server(tcp_port)
	explicit forum_server(const unsigned int tcp_port = 0) : http::server(tcp_port)
	{
		set_logger(PION_GET_LOGGER("cppforo.forum_server"));
	}

	virtual void handle_request(http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn, const boost::system::error_code& ec);
};