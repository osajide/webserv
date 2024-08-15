#include "../inc/server.hpp"
#include <fstream>
#include <arpa/inet.h> // temp for inet_addr()
// #include <sys/_types/_fd_def.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>

std::vector<config>    								server::_config;
std::vector<std::pair<std::string, std::string> >	server::_bound_addresses;

server::server()
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
			server_names = server::_config[i].fetch_directive_value("server_name");
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
	std::cout << "closing fd " << this->_clients[client_index].get_fd() << std::endl;

	// FD_CLR(this->_clients[client_index].get_fd(), &read_fds);
	FD_CLR(this->_clients[client_index].get_fd(), &set_fd.read_fds);

	// if (FD_ISSET(this->_clients[client_index].get_fd(), &write_fds))
	if (FD_ISSET(this->_clients[client_index].get_fd(), &set_fd.write_fds))
		FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);

	close(this->_clients[client_index].get_fd());
	this->_clients.remove_from_begin(client_index);
}

void    server::parse_config(char *PathToConfig)
{
    std::fstream	    file;
	std::string			reader;

	file.open(PathToConfig);
	if (file.is_open())
	{
    	while (getline(file, reader))
		{
			if (reader == "server")
				server::_config.push_back(config(file));
		}
		file.close();
	}
}

int	server::if_ip_port_already_bound(std::string ip, std::string port)
{
	for (size_t i = 0; i < server::_bound_addresses.size(); i++)
	{
		if (ip == server::_bound_addresses[i].first
				&& port == server::_bound_addresses[i].second)
		{
				return (1);
		}
	}
	return (0);
}

server::server(int conf_index) : _conf_index(conf_index)
{
    int	sock = socket(AF_INET, SOCK_STREAM,0);
	if (sock == -1)
	{
		perror("Error in creating a socket");
		throw 1;
	}

	this->_fd = sock;

	std::vector<std::string>	listenDirective;
	size_t						pos;

	listenDirective = server::_config[_conf_index].fetch_directive_value("listen");

	pos = listenDirective.front().find(':');
	this->_ip = listenDirective.front().substr(0, pos);
	this->_port = listenDirective.front().substr(pos + 1, listenDirective.size() - (pos + 1));

	this->_addr.sin_family = AF_INET;
	this->_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
	this->_addr.sin_port = htons(atoi(_port.c_str()));

    this->init_socket();
}

void	server::init_socket()
{
	if (server::if_ip_port_already_bound(this->_ip, this->_port))
		return ;

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
    std::vector<std::string>	root_directive_inside_location;
    std::string					full_path;

	
	if (location_index == -1)
		full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();
	else
	{
		root_directive_inside_location = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "root");

		if (root_directive_inside_location.size() == 0)
            full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();

		else
			full_path = root_directive_inside_location.front();

	}

	full_path += this->_clients[client_index]._request.get_target();
	// std::cout << "full path =====----> '" << full_path << "'" << std::endl;

	if (access(full_path.c_str(), F_OK) == -1)
		return ("");

	return (full_path);
}

void    server::check_if_method_allowed_in_location(int client_index, int location_index)
{
    int				            allowed;
	LocationPair				location_block;
    std::vector<std::string>    allowed_methods;
	
	allowed = 0;
	if (location_index == -1)
	{
		allowed_methods = server::_config[this->_conf_index].fetch_directive_value("allowed_methods");

		for (size_t i = 0; i < allowed_methods.size(); i++)
		{
			if (this->_clients[client_index]._request.get_method() == allowed_methods[i])
			{
				// std::cout << "ALLOWED FROM THE SERVER CONF NOT LOCATION" << std::endl;
				allowed = 1;
			}
		}
	}
	else
	{
        allowed_methods = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "allowed_methods");

        for (size_t i = 0; i < allowed_methods.size(); i++)
        {
            if (this->_clients[client_index]._request.get_method() == allowed_methods[i])
            {
                // std::cout << "ALLOWEEEEEEED!!!!!" << std::endl;
                allowed = 1;
            }
        }
	}
	if (allowed == 0)
		throw 405; // 405 Method Not Allowed
}

int	server::check_resource_type(std::string path)
{
	struct stat	path_stat;

	stat(path.c_str(), &path_stat);

	if (S_ISREG(path_stat.st_mode))
	{
		// std::cout << "----------> The path is a regular file" << std::endl;
		return (REG_FILE);
	}
	if (S_ISDIR(path_stat.st_mode))
	{
		// std::cout << "==========> The path is a directory" << std::endl;
		return (DIRECTORY);
	}
	return (-1); // In case of symbolic link or other types
}

void    server::handle_request(int client_index, fd_sets& set_fd, int location_index)
{
	std::string	path;

	this->check_if_method_allowed_in_location(client_index, location_index);
	
	path = this->check_availability_of_requested_resource(client_index, location_index);
	
	if (path.empty()) // in the previous function if the path doesn't exist i return an empty string
	{
		this->_clients[client_index]._response.return_error(404, this->_clients[client_index].get_fd());
		std::cout << "404 SERVED!!!" << std::endl;
		std::cout << "clearing fd " << this->_clients[client_index].get_fd() << " from write_fds" << std::endl;
		// FD_CLR(this->_clients[client_index].get_fd(), &write_fds);
		FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
	}
	else
	{
		int path_check = this->check_resource_type(path);
		if (path_check == DIRECTORY)
		{
			if (this->_clients[client_index]._request.get_target().back() == '/')
			{
				if (this->_clients[client_index].dir_has_index_files(path)) // method takes reference to path to change it in case of not finding an index
				{
					if (this->_clients[client_index].if_cgi_directive_exists())
					{
						// if method == GET , run cgi on requested file with GET REQUEST_METHOD
						// if method == POST , run cgi on requested file with POST REQUEST_METHOD
					}
					else
					{
						if (this->_clients[client_index]._request.get_method() == "POST")
						{
							std::cout << "before throw 403" << std::endl;
							throw 403; // Forbidden
						}

						this->_clients[client_index]._response.set_path_to_serve(path);
						this->_clients[client_index].set_ready_for_receiving_value(true);
					}
				}
				// else
				// {
				// 	if (this->_clients[client_index]->get_request().get_method() == "POST")
				// 	{
				// 		std::cout << "before throw 403**" << std::endl;
				// 		throw 403; // Forbidden
				// 	}
					
				// 	int	autoindex_check = this->_config.fetch_autoindex_value(location_index);
				// 	if (autoindex_check == ON)
				// 	{
				// 		this->return_requested_directory(200, client_index, location_index, path, ON);
				// 		std::cout << "autoindex served !!" << std::endl;
				// 	}
				// }
			}
			else
			{
				// std::cout << "redirection!!!!!!!!!!!!!" << std::endl;
				// std::cout << "path sent to the function = '" << path << "'" << std::endl;
				// this->return_requested_directory(301, client_index, location_index, path, OFF);
				// this->_clients[client_index]._response.redirect();
			}
		}
		else if (path_check == REG_FILE)
		{
			if (this->_clients[client_index].if_cgi_directive_exists())
			{
				// if method == GET , run cgi on requested file with GET REQUEST_METHOD
				// if method == POST , run cgi on requested file with POST REQUEST_METHOD
			}
			else
			{
				if (this->_clients[client_index]._request.get_method() == "POST")
					throw 403; // Forbidden

				this->_clients[client_index].set_ready_for_receiving_value(true);
				this->_clients[client_index]._response._path_to_serve = path;

			}
		}
	}
}
