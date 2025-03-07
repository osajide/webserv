#include "../inc/server.hpp"
#include <fstream>
#include <arpa/inet.h> // temp for inet_addr()
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include "../inc/error.hpp"
#include "../inc/webserv.hpp"

std::vector<config>																		server::_config;
std::vector<std::pair<std::pair<std::string, std::string>, std::vector<std::string> > >	server::_bound_addresses;

server::server() : _bound(false)
{}

int	server::match_server_name(int server_conf_index, std::string server_name_to_match)
{
	std::vector<std::string> server_names;
	std::string				 listen_directive;

	listen_directive = server::_config[server_conf_index].fetch_directive_value("listen").front();

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		if (listen_directive == server::_config[i].fetch_directive_value("listen").front())
		{
			server_names = server::_config[i].fetch_directive_value("server_names");
			if (!server_names.empty())
			{
				for (size_t j = 0; j < server_names.size(); j++)
				{
					if (server_name_to_match == server_names[j])
						return (i);
				}
				server_names.clear();
			}
		}
	}
	return (server_conf_index); // if no server name matched just return the server conf index
}

void	server::close_connection(int client_index, fd_sets & set_fd)
{

	FD_CLR(this->_clients[client_index].get_fd(), &set_fd.read_fds);
	FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);

	close(this->_clients[client_index].get_fd());

	this->_clients.remove_from_begin(client_index);
}

void	server::parse_config(const char *PathToConfig)
{
	std::fstream				file;
	std::string					reader;
	std::vector<std::string>	listen_directive;

	server::run_check(PathToConfig);

	file.open(PathToConfig);
	if (file.is_open())
	{
		while (getline(file, reader))
		{
			if (reader == "server")
				server::_config.push_back( (file));
		}
		file.close();
	}

	webserv::set_status_lines();
	config::set_dictionary();

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		server::_config[i].check_validity_of_global_directives();
		server::_config[i].check_validity_of_location_directives();
		server::_config[i].check_for_conflicts_and_set_default_values();
		server::_config[i].check_presence_of_mandatory_directives();
	}

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		listen_directive = server::_config[i].fetch_directive_value("listen");
		if (listen_directive.size() > 1) // i.e we have more than one ip/port
		{
			for (size_t j = 1; j < listen_directive.size(); j++) // j = 1, to skip the first ip/port
			{
				config	new_conf_block(server::_config[i], listen_directive[j]);
				server::_config.push_back(new_conf_block);
			}
		}
	}
}

int	server::if_ip_port_already_bound(std::string ip, std::string port)
{
	for (size_t i = 0; i < server::_bound_addresses.size(); i++)
	{
		if (ip == server::_bound_addresses[i].first.first && port == server::_bound_addresses[i].first.second)
		{
			return (i);
		}
	}
	return (-1);
}

server::server(int conf_index) : _bound(false), _conf_index(conf_index)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;

	int    sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("Error in creating a socket");
		throw 1;
	}

	this->_fd = sock;

	std::vector<std::string>    listenDirective;
	size_t                        pos;

	listenDirective = server::_config[_conf_index].fetch_directive_value("listen");

	pos = listenDirective.front().find(':');
	this->_ip = listenDirective.front().substr(0, pos);
	this->_port = listenDirective.front().substr(pos + 1, listenDirective.size() - (pos + 1));

	this->_addr.sin_family = AF_INET;
	
	memset(&hints, 0, sizeof hints); // Initialize the structure
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP

	if (getaddrinfo(_ip.c_str(), NULL, &hints, &res) != 0)
		throw 1;

	struct sockaddr_in *sockaddr = (struct sockaddr_in *)res->ai_addr;
	
	this->_addr.sin_addr.s_addr = sockaddr->sin_addr.s_addr;
	freeaddrinfo(res);

	this->_addr.sin_port = htons(ft_atol(_port.c_str()));


	this->init_socket();
}

void	server::init_socket()
{
	int	bound_address_index;

	bound_address_index = server::if_ip_port_already_bound(this->_ip, this->_port);
	if (bound_address_index != -1)
	{	
		std::vector<std::string> server_names;
		server_names = this->_config[this->_conf_index].fetch_directive_value("server_names");
		if (server_names.empty())
			std::cout << "webserv: [warn] conflicting server name \"\" on " << this->_ip << ":" << this->_port << ", ignored" << std::endl;
		else
		{
			for (size_t i = 0; i < server_names.size(); i++)
			{
				size_t j = 0;
				for (; j < server::_bound_addresses[bound_address_index].second.size(); j++)
				{
					if (server_names[i] == server::_bound_addresses[bound_address_index].second[j])
					{
						std::cout << "webserv: [warn] conflicting server name \"" << server_names[i] << "\" on " << this->_ip << ":" << this->_port << ", ignored" << std::endl;
						break;
					}
				}
				if (j == server::_bound_addresses[bound_address_index].second.size())
				{
					server::_bound_addresses[bound_address_index].second.push_back(server_names[i]);
				}
			}
		}
		return ;
	}

	int	reuseaddr_enable;

	reuseaddr_enable = 1;
	if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_enable, sizeof(reuseaddr_enable)) == -1)
	{
		perror("Error in setting socket option");
		close(this->_fd);
		throw 1;
	}
	if (bind(this->_fd, (struct sockaddr *)&this->_addr, sizeof(this->_addr)) == -1)
	{
		perror("Error binding socket");
		close(this->_fd);
		throw 1;
	}
	if (listen(this->_fd, SOMAXCONN) == -1)
	{
		perror("Error in listening");
		close(this->_fd);
		throw 1;
	}

	// server::_bound_address is a vector of a pair of pair and vector of strings
	server::_bound_addresses.push_back(std::make_pair(std::make_pair(this->_ip, this->_port), this->_config[this->_conf_index].fetch_directive_value("server_names")));
	this->_bound = true;
}

int server::get_fd()
{
    return (this->_fd);
}

int server::get_config_index()
{
    return (this->_conf_index);
}

std::string server::check_availability_of_requested_resource(int client_index, int location_index)
{
	int							alias_check;
	std::string					target;
	std::vector<std::string>	root_directive_inside_location;
	std::vector<std::string>	alias_directive_inside_location;
	std::string					full_path;

	
	alias_check = 0;
	if (location_index == -1)
	{
		full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();
	}
	else
	{

		alias_directive_inside_location = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "alias");
		if (alias_directive_inside_location.empty())
		{
			root_directive_inside_location = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "root");
			if (root_directive_inside_location.empty())
			{
				full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();
			}
			else
				full_path = root_directive_inside_location.front();
		}
		else
		{
			full_path = alias_directive_inside_location.front();
			alias_check = 1;
		}
	}

	if (alias_check == 1)
	{
		std::string location_name = server::_config[this->_clients[client_index]._config_index]._locations[location_index].first;
		size_t pos = this->_clients[client_index]._request._target.find(location_name);

		target = this->_clients[client_index]._request._target.substr(pos + location_name.length());
	}
	else
		target = this->_clients[client_index]._request._target;

	std::cout << "target = " << target << std::endl;
	full_path += target;
	std::cout << "full path =====----> '" << full_path << "'" << std::endl;
	// exit(1);
	if (access(full_path.c_str(), F_OK) == -1)
		return ("");

	return (full_path);
}

std::string    server::check_if_method_allowed_in_location(int client_index, int location_index)
{
	LocationPair				location_block;
	std::vector<std::string>    allowed_methods;

	if (location_index == -1)
	{
		allowed_methods = server::_config[this->_conf_index].fetch_directive_value("allowed_methods");

		if (allowed_methods.empty())
			return ("GET");
		for (size_t i = 0; i < allowed_methods.size(); i++)
		{
			if (this->_clients[client_index]._request._method == allowed_methods[i])
			{
				// std::cout << "ALLOWED FROM THE SERVER CONF NOT LOCATION" << std::endl;
				return (allowed_methods[i]);
			}
		}
	}
	else
	{
		if (!server::_config[this->_conf_index].directive_inside_location_exists(location_index, "allowed_methods"))
			return ("GET");

		allowed_methods = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "allowed_methods");

		for (size_t i = 0; i < allowed_methods.size(); i++)
		{
			if (this->_clients[client_index]._request._method == allowed_methods[i])
			{
				// std::cout << "ALLOWEEEEEEED!!!!!" << std::endl;
				return (allowed_methods[i]);
			}
		}
	}
	throw error(405, client_index); // 405 Method Not Allowed
}

int	server::check_resource_type(std::string path)
{
	struct stat	path_stat;

	stat(path.c_str(), &path_stat);

	if (S_ISDIR(path_stat.st_mode))
	{
		return (DIRECTORY);
	}
	return (REG_FILE);
}

void    server::handle_request(int client_index, fd_sets& set_fd, int location_index, char** env)
{
	std::string	req_method;

	req_method = this->check_if_method_allowed_in_location(client_index, location_index);
	
	this->_clients[client_index]._response._path_to_serve = this->check_availability_of_requested_resource(client_index, location_index);
	
	if (this->_clients[client_index]._response._path_to_serve.empty()) // in the previous function if the path doesn't exist i return an empty string
	{
		this->_clients[client_index]._response.return_error(webserv::get_corresponding_status(404), this->_clients[client_index].get_fd());
		std::cout << "404 SERVED!!!" << std::endl;
		
		FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
		
		if (this->_clients[client_index]._request._headers["Connection"] == "closed")
			throw error(CLOSE_CONNECTION, client_index);
		
		this->_clients[client_index].clear_client();
	}
	else
	{
		if (this->check_resource_type(this->_clients[client_index]._response._path_to_serve) == DIRECTORY)
		{
			if (req_method == "DELETE")
			{
				this->_clients[client_index].handle_delete_directory_request(set_fd, env);
				return ;
			}
			if (this->_clients[client_index]._request._target[this->_clients[client_index]._request._target.length() - 1] == '/')
			{
				if (this->_clients[client_index].dir_has_index_files()) // this method modifies on path_to_serve attribute
				{
					if (server::_config[this->_clients[client_index]._config_index].if_cgi_directive_exists(this->_clients[client_index]._location_index, this->_clients[client_index]._response._path_to_serve))
					{
						this->_clients[client_index]._cgi._cgi_processing = true;
					}
					else
					{
						if (req_method == "POST")
						{
							std::cout << "cgi not found" << std::endl;
							throw error(403, client_index); // Forbidden
						}

						this->_clients[client_index]._response._status_code = 200;
						this->_clients[client_index].set_ready_for_receiving_value(true);
					}
				}
				else
				{
					int autoindex_check = server::_config[this->_clients[client_index]._config_index].fetch_autoindex_value(location_index);

					if (req_method == "POST" || autoindex_check != ON)
					{
						std::cout << "before throw 403**" << std::endl;
						throw error(403, client_index); // Forbidden
					}
					else
					{
						this->_clients[client_index]._response.autoindex(this->_clients[client_index].get_fd(), this->_clients[client_index]._request._target);
						FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
						
						if (this->_clients[client_index]._request._headers["Connection"] == "closed")
							throw error(CLOSE_CONNECTION, client_index);

						this->_clients[client_index].clear_client();
						std::cout << "autoindex served !!" << std::endl;
					}
				}
			}
			else
			{
				if (req_method == "DELETE")
					throw error(409, client_index); // Conflict

				this->_clients[client_index]._response.redirect(this->_clients[client_index].get_fd(), 301, this->_clients[client_index]._request._target + '/');
				
				FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);

				if (this->_clients[client_index]._request._headers["Connection"] == "closed")
					throw error(CLOSE_CONNECTION, client_index);

				this->_clients[client_index].clear_client();
			}
		}
		else
		{
			if (server::_config[this->_clients[client_index]._config_index].if_cgi_directive_exists(this->_clients[client_index]._location_index, this->_clients[client_index]._response._path_to_serve))
			{
				this->_clients[client_index]._cgi._cgi_processing = true;
			}
			else
			{
				std::string	method = req_method;

				if (method == "POST")
					throw error(403, client_index); // Forbidden

				else if (method == "DELETE")
				{
					this->_clients[client_index]._response.remove_requested_file(this->_clients[client_index].get_fd());

					FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
					
					if (this->_clients[client_index]._request._headers["Connection"] == "closed")
						throw error(CLOSE_CONNECTION, client_index);

					this->_clients[client_index].clear_client();
				}
				else
				{
					this->_clients[client_index]._response._status_code = 200;
					this->_clients[client_index].set_ready_for_receiving_value(true);
				}

			}
		}
	}
}




/*


HTTP/1.1 413 Request Entity Too Large
Content-Type: 
Content-Length: 37

<h1>413 Request Entity Too Large</h1>


*/