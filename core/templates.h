#include <map>
#include <string>;
//#include <regex>;
#include <fstream>;
#include <boost/regex.hpp>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include "database.h"

class TemplateNode
{
public:
	//TemplateNode * childNode = nullptr;
	std::map<std::string, TemplateNode*> childNodes;
	std::string body;

	TemplateNode(std::string name, TemplateNode * node);
	TemplateNode(std::string * data);
	TemplateNode(char * data);
};

class Templates
{
	std::map<std::string, std::string> templateSet;
	std::map<std::string, TemplateNode*> tmplmap;
	//sql::Connection * con;
	Database * db;
	sql::PreparedStatement * tmplStatement;
public:
	//Templates(sql::Connection * _con);
	Templates(Database * _db);
	bool loadTemplate(std::string name);
	std::string getTemplate(std::string name);
	std::string render(std::string name);
	void assignVar(std::string name, std::string content);
	void build(std::string);
};