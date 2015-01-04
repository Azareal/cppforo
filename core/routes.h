#include <map>
#include <functional>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <pion/config.hpp>
#include <pion/error.hpp>
#include <pion/tcp/connection.hpp>
#include <pion/http/request.hpp>
#include <pion/http/request_reader.hpp>
#include <pion/http/response_writer.hpp>

#include <openssl/blowfish.h>

#include "templates.h"
#include "forum.h"
#include "user.h"
#include "parse.h"

using namespace pion;

typedef std::function<void(std::string, http::request_ptr& http_request_ptr, tcp::connection_ptr& tcp_conn)> Route;
typedef std::map<std::string, Route> RouteMap;

void addRoute(std::string name, Route route);
void init_default_routes();
void membersonly_error(http::response_writer_ptr& writer);
void banned_error(http::response_writer_ptr& writer);
