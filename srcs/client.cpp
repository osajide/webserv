#include "../inc/client.hpp"
#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "../inc/server.hpp"


client::client()
{}

client::client(int client_sock, int conf_index, int index) : _index(index), _config_index(conf_index), _location_index(-2), _fd(client_sock), _ready_for_receiving(false), _read_body(false),
													_max_body_size(0), _bytes_read(0)
{}

client::client(client const &rhs) : _request(rhs._request), _response(rhs._response), _cgi(rhs._cgi)
{
	*this = rhs;
}

client&	client::operator=(const client &rhs)
{
	if (this != &rhs)
	{
		this->_index = rhs._index;
		this->_request = rhs._request;
		this->_response = rhs._response;
		this->_config_index = rhs._config_index;
		this->_location_index = rhs._location_index;
		this->_fd = rhs._fd;
		this->_ready_for_receiving = rhs._ready_for_receiving;
		this->_read_body = rhs._read_body;
		this->_max_body_size = rhs._max_body_size;
		this->_bytes_read = rhs._bytes_read;
	}
	return (*this);
}

int client::get_fd()
{
    return (this->_fd);
}

void	client::convert_numeric_values()
{
	this->_max_body_size = std::atoi(server::_config[this->_config_index].fetch_directive_value("client_max_body_size").front().c_str());
	this->_request._content_length = std::atoi(this->_request._headers["Content-Length"].c_str());
}

void    client::fill_request_object()
{
    std::stringstream	ss(this->_request._raw_request);
	std::string			reader;
	std::string			key;
	std::string			value;
	size_t				pos;

	getline(ss, reader);
	this->_request.set_request_line(reader, this->_index);

	while (getline(ss, reader))
	{
		for (size_t i = 0; i < reader.size(); i++)
		{
			if (reader[i] == '\r')
				reader.erase(i, 1);
		}		
		pos = reader.find(':');
		if (pos != reader.npos)
		{
			key = reader.substr(0, pos);
			value = reader.substr(pos + 2, reader.size());
			this->_request.set_header(key, value);
		}
	}
	this->convert_numeric_values();
}

int	client::dir_has_index_files()
{
	std::vector<std::string>	index_files;
	std::string					access_path;

	if (this->_location_index == -1)
	{
		index_files = server::_config[this->_config_index].fetch_directive_value("index");
	}
	else
	{
		if (server::_config[this->_config_index].directive_inside_location_exists(this->_location_index, "index"))
		{
			index_files = server::_config[this->_config_index].fetch_location_directive_value(this->_location_index, "index");
		}
		else
		{
			index_files = server::_config[this->_config_index].fetch_directive_value("index");
		}
	}


	for (size_t i = 0; i < index_files.size(); i++)
	{
		access_path = this->_response._path_to_serve + index_files[i];
		std::cout << "access path = '" << access_path << "'" << std::endl;
		if (access(access_path.c_str(), F_OK) == 0) // Default index
		{
			this->_response._path_to_serve = access_path;
			return (1);
		}
		access_path.clear();
	}
	return (0);
}

void	client::does_location_has_redirection()
{
	if (this->_location_index == -1)
	{
		return ;
	}
	else
	{
		std::vector<std::string> return_directive;

		return_directive = server::_config[this->_config_index].fetch_location_directive_value(this->_location_index, "return");
		if (!return_directive.empty())
		{
			this->_response._redirection_path = return_directive.back();
			std::cout << "path to redirect = '" << this->_response._redirection_path << "'" << std::endl;
			throw error(atoi(return_directive.front().c_str()), this->_index);
		}
	}
}

size_t	hex_to_decimal(std::string hex)
{
	size_t				decimal_val;
	std::stringstream	ss;

	ss << std::hex << hex;
	ss >> decimal_val;

	return (decimal_val);
}

size_t	client::unchunk_rest_of_raw_body()
{
	size_t				chunk_size;
	char				buffer[BUFFER_SIZE + 1];
	std::string			reader;
	std::stringstream	ss(this->_request._raw_body);

	memset(buffer, 0, BUFFER_SIZE + 1);

	getline(ss, reader);
	chunk_size = hex_to_decimal(reader);

	if (chunk_size > BUFFER_SIZE)
		ss.read(buffer, BUFFER_SIZE);
	else
		ss.read(buffer, chunk_size);

	this->_unchunked_body_file << buffer;

	if ((size_t)ss.gcount() < chunk_size)
	{
		return (chunk_size);
	}
	return (0);
}

void	client::read_chunked_body(fd_sets& set_fd)
{
	int			valread = 0;
	char		buffer[BUFFER_SIZE + 1];
	std::string	reader;
	size_t		chunk_size;
	
	// first phase put all chunked body in a file and after that unchunk it
	if (!this->_unchunked_body_file.is_open())
	{
		if (!this->_body_file.is_open())
		{
			this->_body_file.open(this->_cgi.get_random_file_name(this->_index, INPUT_FILE), std::ios::app);
		}
		else
		{
			if (!this->_request._raw_body.empty())
			{
				this->_body_file << this->_request._raw_body;
				if (this->_request._raw_body.rfind("0\r\n\r\n") != this->_request._raw_body.npos)
				{
					this->_cgi._infile = this->_cgi.get_random_file_name(this->_index, INPUT_FILE);
					this->_unchunked_body_file.open(this->_cgi._infile, std::ios::app);
				}
				this->_request._raw_body.clear();
			}
			else
			{
				memset(buffer, 0, BUFFER_SIZE + 1);
				valread = read(this->_fd, buffer, BUFFER_SIZE);
				if (valread == 0 || valread == -1)
					throw error(-1, this->_index);
				// std::cout << "valread = " << valread << std::endl;
				// std::cout << "buffer read from body ---------- :" << std::endl;
				// std::cout << buffer << std::endl;
				// std::cout << "--------" << std::endl;
				sleep(2);

				this->_body_file << buffer;
				std::string	str_buffer(buffer);
				if (str_buffer.rfind("0\r\n\r\n") != str_buffer.npos)
				{
					this->_cgi._infile = this->_cgi.get_random_file_name(this->_index, INPUT_FILE);
					this->_unchunked_body_file.open(this->_cgi._infile, std::ios::app);
				}
			}
		}
	}
	if (this->_unchunked_body_file.is_open())
	{
		getline(this->_body_file, reader);
		size_t pos = reader.rfind('\r');
		if (pos != reader.npos)
		{
			reader.erase(pos, 1);
		}
		chunk_size = hex_to_decimal(reader);
		char	buffer[chunk_size + 1];
		memset(buffer, 0, chunk_size + 1);
		this->_body_file.read(buffer, chunk_size);
		this->_request._content_length += chunk_size;
		std::string	str_buffer(buffer);
		if (str_buffer.rfind("0\r\n\r\n") != str_buffer.npos)
		{
			this->_unchunked_body_file.close();
			this->_body_file.close();
			FD_SET(this->_fd, &set_fd.write_fds);
		}
	}
}

void	client::read_body_based_on_content_length(fd_sets& set_fd)
{
	int		valread = 0;
	char	buffer[BUFFER_SIZE + 1];

	if (this->_cgi._infile.empty())
	{
		this->_cgi._infile = this->_cgi.get_random_file_name(this->_index, INPUT_FILE);
		std::cout << "infile = " << this->_cgi._infile << std::endl;
		this->_body_file.open(this->_cgi._infile, std::ios::app);
	}
	if (this->_body_file.is_open())
	{
		std::cout << "bytes read = " << this->_bytes_read << std::endl;
		std::cout << "content le = " << this->_request._content_length << std::endl;
		if (this->_bytes_read < this->_request._content_length)
		{
			if (this->_request._raw_body.empty())
			{
				memset(buffer, 0, BUFFER_SIZE + 1);
				valread = read(this->_fd, buffer, BUFFER_SIZE);
				if (valread == 0 || valread == -1)
					throw error(-1, this->_index);

				// std::cout << "valread = " << valread << std::endl;
				// std::cout << "buffer read from body ---------- :" << std::endl;
				// std::cout << buffer << std::endl;
				// std::cout << "--------" << std::endl;
				sleep(2);

				this->_body_file << buffer;
				this->_bytes_read += valread;
			}
			else
			{
				this->_body_file << this->_request._raw_body;
				std::cout << "raw body size = " << this->_request._raw_body.length() << std::endl;
				this->_bytes_read += this->_request._raw_body.length();
				this->_request._raw_body.clear();
			}
		}
		if (this->_bytes_read == this->_request._content_length)
		{
			this->_body_file.close();
			FD_SET(this->_fd, & set_fd.write_fds);
		}
	}
}

void    client::read_request(int conf_index, fd_sets & set_fd)
{
	size_t		pos;
	int			valread = 0;
	char		buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, BUFFER_SIZE + 1);

	if (this->_read_body == false)
	{
		valread = read(this->_fd, buffer, BUFFER_SIZE);

		if (valread == 0 || valread == -1)
			throw error(-1, this->_index);

		std::cout << "------------------ valread = " << valread << std::endl;
		std::cout << "------ buffer read from fd  " << this->_fd << ":" << std::endl;
		std::cout << buffer << std::endl;
		std::cout << "-----------------" << std::endl;

		this->_request._raw_request += buffer;
		pos = this->_request._raw_request.find("\r\n\r\n");

		if (pos != this->_request._raw_request.npos)
		{
			this->fill_request_object();
			this->_request.is_well_formed(this->_index, server::_config[conf_index]);

			this->_config_index = server::match_server_name(this->_config_index, this->_request.fetch_header_value("host"));
			this->_location_index = this->_request.does_uri_match_location(server::_config[conf_index].get_locations(), this->_request._target);
            
        	this->does_location_has_redirection();

			if (this->_request.header_exists("Transfer-Encoding") || this->_request.header_exists("Content-Length"))
			{
				std::cout << "max body size = " << this->_max_body_size << std::endl;
				if (this->_request.header_exists("Content-Length"))
				{
					std::stringstream	s(this->_request._headers["Content-Length"]);
					s >> this->_request._content_length;
					if (this->_request._content_length == 0)
					{
						FD_SET(this->_fd, &set_fd.write_fds);
						this->_read_body = false;
					}
					else
						this->_read_body = true;
				}

			}
			else
			{
				FD_SET(this->_fd, &set_fd.write_fds);
				this->_read_body = false;
			}
		}
	}
	if (this->_read_body == true)
	{
		if (buffer[0] != '\0') // In case this is the first time entering this block
		{
			this->_request._raw_body = this->_request._raw_request.substr(pos + 4);
		}
		if (this->_request.header_exists("Transfer-Encoding")) // handle_chunked_body
		{
			this->read_chunked_body(set_fd);
		}
		else
		{
			this->read_body_based_on_content_length(set_fd);
		}
	}
}

void	client::handle_delete_directory_request(fd_sets& set_fd)
{
	if (server::_config[this->_config_index].if_cgi_directive_exists(this->_location_index, this->_request._target))
	{
		if (this->dir_has_index_files())
		{
			// run cgi on requested file with DELETE REQUESTED_METHOD
		}
		else
			this->_response.return_error(403, this->_fd);
	}
	else
	{
		this->_response.remove_requested_directory(this->_fd, this->_response._path_to_serve);
	}

	this->clear_client();
	FD_CLR(this->_fd, &set_fd.write_fds);
}

void    client::set_ready_for_receiving_value(bool value)
{
    this->_ready_for_receiving = value;
}

bool&    client::get_ready_for_receiving_value()
{
    return (this->_ready_for_receiving);
}

void	client::clear_client()
{
	this->_ready_for_receiving = false;
	this->_location_index = -2;
	this->_bytes_read = 0;
	this->_read_body = false;
	this->_max_body_size = 0;

	if (this->_body_file.is_open())
		this->_body_file.close();

	this->_request.clear_request();
	this->_response.clear_response();
	this->_cgi.clear_cgi();
}
