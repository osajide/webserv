#include "../inc/response.hpp"
#include "../inc/autoindex.hpp"
#include "../inc/webserv.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>

response::response() : _bytes_sent(0), _status_line(""), _content_length(0), _content_type(""), _location(""), _body(""), _headers(""),
						_chunk(""), _bytes_written(0), _unsent_part(""), _redirection_path(""), _status_code(0)
{}

response::response(response const & rhs)
{
	*this = rhs;
}

response &	response::operator=(response const & rhs)
{
	if (this != &rhs)
	{
		this->_body = rhs._body;
		this->_bytes_sent = rhs._bytes_sent;
		this->_bytes_written = rhs._bytes_written;
		this->_unsent_part = rhs._unsent_part;
		this->_chunk = rhs._chunk;
		this->_content_length = rhs._content_length;
		this->_content_type = rhs._content_type;
		this->_headers = rhs._headers;
		this->_location = rhs._location;
		this->_status_line = rhs._location;
	}
	return (*this);
}

void    response::set_status_line(std::string status_line)
{
    this->_status_line = status_line;
}

void    response::set_content_length(std::ifstream& requested_file)
{
	requested_file.seekg(0, std::ios::end);

	this->_content_length = requested_file.tellg();

	requested_file.seekg(0, std::ios::beg);
}

std::string	response::get_chunk(std::ifstream& requested_file)
{
	std::string	chunk;
	char		buffer[5000];

	memset(buffer, 0, 5000);

	requested_file.read(buffer, 4999);

	chunk.assign(buffer, requested_file.gcount());

	return (chunk);
}

void	response::send_reply(int target_fd)
{

	this->_headers += this->_status_line + "\r\n";

	this->_headers += "Content-Type: " + this->_content_type + "\r\n";

	std::stringstream	ss;

	ss << this->_content_length;

	this->_headers += "Content-Length: " + ss.str() + "\r\n";

	if (!this->_location.empty())
		this->_headers += "Location: " + this->_location + "\r\n";

	this->_headers += "\r\n";
	if (!this->_body.empty())
		this->_headers += this->_body;

	write(target_fd, this->_headers.c_str(), this->_headers.length());
}

void	response::return_error(std::string status_line, int target_fd)
{
	this->_status_line = status_line;
	this->_body = "<h1>" + status_line.substr(9) + "</h1>";
	this->_content_length = this->_body.length();

	this->send_reply(target_fd);
	this->clear_response();
}

void	response::send_response(int fd, std::string status_line, config serverConf, time_t & client_connection_time)
{
	if (!this->_requested_file.is_open())
	{
		this->_status_line = status_line;
		this->_requested_file.open(this->_path_to_serve.c_str());
		this->set_content_length(this->_requested_file);
		this->_content_type = serverConf.fetch_mime_type_value(this->_path_to_serve);

		this->send_reply(fd);
		client_connection_time = time(NULL);
	}
	else if (this->_requested_file.is_open())
	{
		if (this->_bytes_sent < this->_content_length)
		{
			if (this->_bytes_written == 0)
			{
				this->_chunk = this->get_chunk(this->_requested_file);
				// std::cout << "chunk ---->> '" << this->_chunk << "'" << std::endl;
				this->_bytes_written = write(fd, this->_chunk.c_str(), this->_chunk.length());
				client_connection_time = time(NULL);
				this->_bytes_sent += this->_bytes_written;
			}

			else if ((size_t)this->_bytes_written < this->_chunk.length())
			{
				long local_write = 0;
				std::cout << "not entirely sent" << std::endl;

				this->_unsent_part = this->_chunk.substr(this->_bytes_written);

				local_write = write(fd, this->_unsent_part.c_str(), this->_unsent_part.length());
				client_connection_time = time(NULL);
				this->_bytes_sent += local_write;
				this->_bytes_written += local_write;
			}

			if ((size_t)this->_bytes_written == this->_chunk.length())
			{
				this->_bytes_written = 0;
				this->_chunk.clear();
				this->_unsent_part.clear();
				std::cout << "chunk sent to fd " << fd << std::endl;
			}
		}
	}
}

void	response::autoindex(int fd, std::string uri)
{
	this->_status_line = "HTTP/1.1 200 OK";
	this->_body = AutoIndex::serve_autoindex(uri, this->_path_to_serve);
	this->_content_length = this->_body.length();

	this->send_reply(fd);
}

void	response::redirect(int fd, int status, std::string uri)
{
	std::stringstream	ss;
	std::string			str_status;

	ss << status;
	ss >> str_status;
	this->_status_line = "HTTP/1.1 " + str_status + " Moved Permanently";
	this->_content_length = 0;
	this->_location = uri;

	this->send_reply(fd);
}

int	response::remove_requested_directory(int fd, std::string uri)
{

	DIR*			directory;
	struct dirent*	entry;
	std::string		str_entry;
	struct stat 	entry_type;


	directory = opendir(uri.c_str());
	if (directory == NULL)
	{
		this->return_error(webserv::get_corresponding_status(403), fd);
		return (-1);
	}

	while ((entry = readdir(directory)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		str_entry = uri;
		str_entry += entry->d_name;

		stat(str_entry.c_str(), &entry_type);

		if (S_ISREG(entry_type.st_mode))
		{
			if (std::remove(str_entry.c_str()) != 0)
			{
				if (access(str_entry.c_str(), R_OK) == 0)
				{
					this->return_error(webserv::get_corresponding_status(500), fd);
				}
				else
					this->return_error(webserv::get_corresponding_status(403), fd);

				closedir(directory);
				return (-1);
			}
		}
		if (S_ISDIR(entry_type.st_mode))
		{
			if (this->remove_requested_directory(fd, str_entry) == -1)
			{
				return (-1);
			}
		}
	}

	std::remove(uri.c_str());
	closedir(directory);

	this->_status_line = "HTTP/1.1 204 No Content";
	this->_body = "<h1>204 No Content</h1>";
	this->_content_length = this->_body.length();

	this->send_reply(fd);

	return (0);
}

void	response::remove_requested_file(int fd)
{
	std::cout << "uri.c_str() = '" << this->_path_to_serve.c_str() << "'" << std::endl;

	if (std::remove(this->_path_to_serve.c_str()) != 0)
	{
		if (access(this->_path_to_serve.c_str(), R_OK) == 0)
		{
			std::cout << "File not removed due to internal error" << std::endl;
			this->return_error(webserv::get_corresponding_status(500), fd);
		}
		else
		{
			std::cout << "Doesn't have write permission" << std::endl;
			this->return_error(webserv::get_corresponding_status(403), fd);
		}
	}
	else
	{
		this->_status_line = "HTTP/1.1 204 No Content";
		this->_body = "<h1>204 No Content</h1>";
		this->_content_length = this->_body.length();

		this->send_reply(fd);
	}
}

void	response::send_cgi_headers(int fd, std::ifstream& requested_file)
{
	std::string	reader;
	std::string	headers;
	size_t		pos;

	while (getline(requested_file, reader))
	{
		std::cout << "reader = " << reader << std::endl;
		reader += '\n';
		headers += reader;
		if (reader == "\r\n")
		{
			std::cout << "dkhel" << std::endl;
			break;
		}
	}
	pos = headers.find("\r\n\r\n");
	write(fd, headers.c_str(), headers.length());

	requested_file.seekg(pos + 4, std::ios::beg);
	std::streamsize curr = requested_file.tellg();
	std::cout << "curr = " << curr << std::endl;
	std::cout << "pos = " << pos << std::endl;
	getline(this->_requested_file, reader);
	std::cout << "r=========>>>>  '" << reader << "'" << std::endl;
	requested_file.seekg(0, std::ios::end);
	std::streamsize endofstream = requested_file.tellg();
	std::cout << "end = " << endofstream << std::endl;

	this->_content_length = endofstream - curr;
	requested_file.seekg(pos + 4, std::ios::beg);
	std::cout << "length ===> " << this->_content_length << std::endl;
}

void	response::clear_response()
{
	this->_status_line.clear();
	this->_content_type.clear();
	this->_content_length = 0;
	this->_location.clear();
	this->_body.clear();
	this->_headers.clear();
	this->_bytes_sent = 0;
	this->_unsent_part.clear();
	this->_chunk.clear();
	this->_bytes_written = 0;
	this->_path_to_serve.clear();
	this->_redirection_path.clear();

	if (this->_requested_file.is_open())
		this->_requested_file.close();
}
