#include "../inc/config.hpp"
#include <sstream>
#include <cstring>

std::vector<std::pair<std::string, std::string> >    config::_mime_types;

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
			// std::cout << "mime first = '" << first << "'" << std::endl;
			// std::cout << "mime second = '" << second << "'" << std::endl;
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

config::config(std::fstream& file)
{
	std::string     line;
	std::string 	token;
	std::string 	key;
	std::string 	value;
	int             cr_count = 0;

	while (getline(file, line))
	{
		// std::cout << "getline = '" << line << "'" << std::endl;
		while (line.find('\t') != std::string::npos)
        {
			line = strtok((char *)line.c_str(), "\t");
            // std::cout << "`Line = '" << line << "'" << std::endl;
        }

		std::stringstream s(line);

		// std::cout << "line = '" << line << "'" << std::endl;
		// if (line == "{" || line == "}")
		// {
		if (line == "{")
			cr_count -= 1;
		else if (line == "}")
			cr_count += 1;
		// }
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
					// std::cout << "line = '" << line << "'" << std::endl;
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
			// std::cout << "key = '" << key << "'" << std::endl;
			while (s >> value)
			{
			// std::cout << "value = '" << value << "'" << std::endl;
				// std::cout << "key = '" << key << "'" << std::endl;
				this->_directives[key].push_back(value.substr(0, value.find(';')));
			}
		}

		if (cr_count == 0)
			break;
	}

	// DirectiveMap::iterator      it;

	// std::cout << "PRINT DIRECTIVES" << std::endl;
	// for (it = this->_directives.begin(); it != this->_directives.end(); it++)
	// {
	// 	std::cout << "key = '" << it->first << "' | value = '" << it->second.front() << "'" << std::endl;
	// }
	// std::cout << "----------------" << std::endl;
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

std::vector<LocationPair>   config::get_locations()
{
    return (this->_locations);
}

LocationPair    config::get_location_block(int location_index)
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
		// std::cout << "extension = '" << extension << "'" << std::endl;
	}

	for (size_t i = 0; i < this->_mime_types.size(); i++)
	{
		if (this->_mime_types[i].second == extension)
			return (this->_mime_types[i].first);
	}
	return ("text/html");
}
