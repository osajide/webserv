#include "../inc/config.hpp"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <sstream>
#include <cstring>
#include "../inc/webserv.hpp"

std::vector<std::pair<std::string, std::string> >	config::_mime_types;
std::vector<std::string>							config::_global_dictionary;
std::vector<std::string>							config::_location_dictionary;

config::config(config const & rhs, std::string ip_port)
{
	this->_locations = rhs._locations;
	this->_directives = rhs._directives;
	
	for (DirectiveMap::iterator	it = this->_directives.begin(); it != this->_directives.end(); it++)
	{
		if (it->first == "listen")
		{
			it->second.clear();
			it->second.push_back(ip_port);
		}
	}
}

void	config::parse_mime_types(const char * PathToMimeTypes)
{
	std::fstream		file;
	std::string			reader;
	std::string			first;
	std::string			second;
	std::stringstream	ss;

	file.open(PathToMimeTypes);
	if (file.is_open())
	{
		while (getline(file, reader))
		{
			ss << reader;
			ss >> first;
			ss >> second;
			_mime_types.push_back(std::make_pair(first, second));
			first.clear();
			second.clear();
			ss.clear();
		}
	}
	else
	{
		perror("Error in opening mime.types");
		exit(EXIT_FAILURE);
	}
}

void	config::set_dictionary()
{
	config::_global_dictionary.push_back("listen");
	config::_global_dictionary.push_back("root");
	config::_global_dictionary.push_back("index");
	config::_global_dictionary.push_back("server_names");
	config::_global_dictionary.push_back("error_page");
	config::_global_dictionary.push_back("client_max_body_size");

	config::_location_dictionary.push_back("root");
	config::_location_dictionary.push_back("allowed_methods");
	config::_location_dictionary.push_back("return");
	config::_location_dictionary.push_back("alias");
	config::_location_dictionary.push_back("autoindex");
	config::_location_dictionary.push_back("index");
	config::_location_dictionary.push_back("cgi");
	config::_location_dictionary.push_back("upload_allowed");
	config::_location_dictionary.push_back("upload_dir");
}

config::config(std::fstream& file)
{
	std::string		line;
	std::string 	token;
	std::string 	key;
	std::string 	value;
	int				cr_count = 0;

	while (getline(file, line))
	{
		while (line.find('\t') != std::string::npos)
		{
			line = strtok((char *)line.c_str(), "\t");
		}

		std::stringstream s(line);

		if (line == "{")
			cr_count -= 1;
		else if (line == "}")
			cr_count += 1;
		else if (line.find("location") != std::string::npos)
		{
			std::stringstream   st(line);
			std::string         location;
			DirectiveMap		location_directives;

			st >> location;
			st >> location; //we do it twice because the first word is "location"

			// we expect that after "location" should come space and not tab

			while (getline(file, line))
			{
				while (line.find('\t') != std::string::npos)
				{
					line = strtok((char *)line.c_str(), "\t");
				}
				if (line == "{")
					cr_count -= 1;
				else if (line == "}")
				{
					cr_count += 1;
					break;
				}
				else
				{
					std::stringstream st(line);

					st >> key;
					while (st >> value)
						location_directives[key].push_back(value.substr(0, value.find(';')));
				}
			}
			_locations.push_back(std::make_pair(location, location_directives));
		}
		else
		{
			s >> key;
			while (s >> value)
			{
				this->_directives[key].push_back(value.substr(0, value.find(';')));
			}
		}
		if (cr_count == 0)
			break;
	}
}

void	config::check_validity_of_global_directives()
{
	std::vector<std::string>::iterator vecIter;

	for (DirectiveMap::iterator it = this->_directives.begin(); it != this->_directives.end(); it++)
	{
		vecIter = std::find(config::_global_dictionary.begin(), config::_global_dictionary.end(), it->first);
		if (vecIter != config::_global_dictionary.end()) // i.e the directive is found
		{
			if (it->first == "error_page")
			{
				if (it->second.size() == 2)
				{
					int status = std::atoi(it->second.front().c_str());
					if (status != 400 && status != 403 && status != 404 && status != 405 && status != 413 && status != 414
							&& status != 500 && status != 501 && status != 504 && status != 505)
						throw "Invalid error code";
				}
				else
					throw "error_page: Invalid number of values";
			}
			else if (it->first == "client_max_body_size")
			{
				if (it->second.size() == 1)
				{
					int max_body_size = std::atoi(it->second.front().c_str());
					if (max_body_size == -1 || max_body_size == 0 || max_body_size > INT_MAX)
						throw "client_max_body_size: Invalid size";
				}
				else
					throw "client_max_body_size: Invalid number of values";
			}
			else if (it->first == "root")
			{
				if (it->second.size() == 1)
				{
					if (it->second[0].length() > 1 && it->second[0][it->second[0].length() - 1] == '/')
						throw "root: Remove '/' from the end";
				}
				else
					throw "root: Invalid number of values";
			}
		}
		else
			throw it->first + ": Directive not known";
	}
}

void	config::check_validity_of_location_directives()
{
	std::vector<std::string>::iterator vecIter;

	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		if (this->_locations[i].first.length() > 1 && this->_locations[i].first[this->_locations[i].first.length() - 1] == '/')
			throw "Remove '/' from the end of location";
		if (this->_locations[i].first[0] != '/')
			throw "Location name must start with '/'";

		for (LocationPair::second_type::iterator it = this->_locations[i].second.begin(); it != this->_locations[i].second.end(); it++)
		{
			vecIter = std::find(config::_location_dictionary.begin(), config::_location_dictionary.end(), it->first);
			if (vecIter != config::_location_dictionary.end()) // i.e the directive is found
			{
				if (it->first == "root")
				{
					if (it->second.size() == 1)
					{
						if (it->second[0][it->second[0].length() - 1] == '/')
							throw "root in location: Remove '/' from the end";
					}
					else
						throw "root in location: Invalid number of values";
				}
				else if (it->first == "allowed_methods")
				{
					if (it->second.size() > 0 && it->second.size() < 4)
					{
						for (size_t j = 0; j < it->second.size(); j++)
						{
							if (it->second[j] != "GET" && it->second[j] != "POST" && it->second[j] != "DELETE")
								throw "allowed_methods: Unknown methods";
						}
					}
				}
				else if (it->first == "return")
				{
					if (it->second.size() == 2)
					{
						if (webserv::status_lines.find(it->second.front()) == webserv::status_lines.end())
							throw "return: Invalid status code";
					}
					else
						throw "return: Invalid number of values";
				}
				else if (it->first == "alias")
				{
					if (it->second.size() == 1)
					{
						if (it->second[0][it->second[0].length() - 1] == '/')
							throw "alias: Remove '/' from the end";
					}
					else
						throw "alias: Invalid number of values";
				}
			}
			else
				throw "Directive inside location not known";
		}
	}
}

void	config::check_presence_of_mandatory_directives()
{
	int							found;

	found = 0;

	for (DirectiveMap::iterator it = this->_directives.begin(); it != this->_directives.end(); it++)
	{
		if (it->first == "listen")
			found++;
		else if (it->first == "client_max_body_size")
			found++;
	}
	if (found != 2)
		throw "Few mandatory directives";
}

int	config::directive_exists(std::string key)
{
	if (this->_directives.find(key) != this->_directives.end())
		return (1);
	return (0);
}

int	config::directive_inside_location_exists(int location_index, std::string key)
{
	LocationPair	location;

	location = this->get_location_block(location_index);
	if (location.second.find(key) != location.second.end())
		return (1);
	return (0);
}

std::vector<std::string>	config::fetch_directive_value(std::string key)
{
	// std::cout << "value = '" << this->_directives[key].front() << "'" << std::endl;
	if (this->_directives.find(key) != this->_directives.end())
		return (this->_directives[key]);

	return (std::vector<std::string>()); // return an empty vector
}

std::vector<LocationPair>	config::get_locations()
{
	return (this->_locations);
}

LocationPair	config::get_location_block(int location_index)
{
	return (this->_locations[location_index]);
}

std::vector<std::string>    config::fetch_location_directive_value(int location_index, std::string key)
{
	LocationPair location_block;

	location_block = this->get_location_block(location_index);
	if (location_block.second.find(key) == location_block.second.end())
	{
		// std::cout << "not foouuuuuuuund" << std::endl;
		return (std::vector<std::string>());
	}
	return (location_block.second[key]);
}

std::string config::fetch_mime_type_value(std::string path)
{
	size_t		pos;
	std::string	extension;

	pos = path.rfind('.');
	if (pos != path.npos)
	{
		extension = path.substr(pos + 1);
	}

	for (size_t i = 0; i < this->_mime_types.size(); i++)
	{
		if (this->_mime_types[i].second == extension)
			return (this->_mime_types[i].first);
	}
	return ("text/html");
}

int	config::fetch_autoindex_value(int location_index)
{
	if (location_index == -1)
	{
		if (this->directive_exists("autoindex"))
		{
			if (this->fetch_directive_value("autoindex").front() == "on")
				return (ON);
		}
	}
	else
	{
		if (this->directive_inside_location_exists(location_index, "autoindex"))
		{
			if (this->fetch_location_directive_value(location_index, "autoindex").front() == "on")
				return (ON);
		}
	}
	return (OFF);
}

std::string	get_extension(std::string path)
{
	std::string	extension;
	size_t		pos;

	pos = path.rfind('.');
	extension = path.substr(pos + 1);
 
	return (extension);
}

int	config::if_cgi_directive_exists(int location_index, std::string path)
{
	std::cout << "path cgi = " << path << std::endl;
	std::string					extension;
	std::vector<std::string>	cgi_directive;

	if (location_index == -1)
		return (0);
	
	extension = get_extension(path);
	cgi_directive = this->fetch_location_directive_value(location_index, "cgi");
	if (!cgi_directive.empty())
	{
		for (size_t i = 0; i < cgi_directive.size(); i++)
		{
			if (extension == cgi_directive[i])
				return (1);
		}
	}
	return (0);
}

std::string	config::error_page_well_defined(int status)
{
	std::stringstream			helper;
	std::string					str_status;
	std::vector<std::string>	error_page_directive;

	helper << status;
	error_page_directive = this->fetch_directive_value("error_page");
	if (!error_page_directive.empty())
	{
		for (size_t i = 0; i < error_page_directive.size(); i += 2)
		{
			// since i can have multiple error_page directive inside i single server block and when adding a new value to an std::map
			// with the same key it overiddes the previous value so i will store the other values of error_pages in the same previous std::vector
			// and since the first element is always the key the second will be the path to the error page so i will loop and increment the i by 2
			// and when matching the same status code i will return the path to the error page
			if (str_status == error_page_directive[i])
			{
				return (error_page_directive[i + 1]);
			}
		}
	}
	return ("");
}