/*
Copyright (c) Azareal 2014.
Licensed under the LGPL v3.
*/

#include "stdafx.h"
#include "templates.h"

std::string regexString = "\\{\\$[a-zA-Z_-][a-zA-Z1-9_-]+\\}";
boost::regex varRegex(regexString);

TemplateNode::TemplateNode(std::string name, TemplateNode * node)
{
	childNodes[name] = node;
}

TemplateNode::TemplateNode(std::string * data)
{
	body = *data;
}

TemplateNode::TemplateNode(char * data)
{
	std::string temp(data);
	body = temp;
}

//Templates::Templates(sql::Connection * _con)
Templates::Templates(ForoDatabase * _db)
{
	db = _db;

	/*try
	{
		tmplStatement = db->con->prepareStatement("SELECT * FROM " + db->prefix + "templates WHERE name = ? LIMIT 1");
	}
	catch (std::exception& e)
	{
		error(e.what());
	}*/
}

bool Templates::loadTemplate(std::string name)
{
	std::string tmpl;

	std::ifstream in("../templates/" + name + ".html", std::ios::in | std::ios::binary);
	if(in)
	{
		in.seekg(0, std::ios::end);
		tmpl.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&tmpl[0], tmpl.size());
		in.close();
		templateSet[name] = tmpl;
		return true;
	}
	/*try
	{
		sql::ResultSet * res;
		tmplStatement->setString(1, name);
		res = tmplStatement->executeQuery();
	
		// Loop over the retrieved settings..
		while (res->next())
		{
			templateSet[res->getString("name")] = res->getString("content");
		}
	
		delete res;
	}
	catch (std::exception& e)
	{
		log(e.what());
		return false;
	}*/

	return false;
}

std::string Templates::getTemplate(std::string name)
{
	return templateSet[name];
}

std::string Templates::render(std::string name)
{
	std::string tmpl = getTemplate(name);
	
	tmpl = boost::regex_replace(tmpl, varRegex, [this](boost::smatch match){
		std::string var = match.str();
		std::string buffer;
		std::string out;
		TemplateNode * lastNode = nullptr;

		for (std::string::iterator it = var.begin() + 2; it != (var.end() - 1); ++it) {
			char c = *it;
			buffer += c;
			if (c == '.')
			{
				if (lastNode == nullptr)
				{
					lastNode = tmplmap[buffer];
					buffer = "";
				}
				else if (lastNode->childNodes[buffer] == nullptr)
				{
					out = lastNode->body;
					buffer = "";
					break;
				}
				else
				{
					lastNode = lastNode->childNodes[buffer];
					buffer = "";
				}
			}
		}

		// Does this template variable exist..?
		if (tmplmap.count(buffer) == 0) return var;

		if (buffer.compare("") != 0) out = tmplmap[buffer]->body;

		return out;
	});

	// Old code.. This is going out the window..
	/*std::sregex_iterator it(tmpl.begin(), tmpl.end(), varRegex);
	std::sregex_iterator it_end;
	while (it != it_end) {
		cout << *it << endl;
		++it;
	}*/

	return tmpl;
}

void Templates::assignVar(std::string name, std::string content)
{
	tmplmap[name] = new TemplateNode(&content);
}