#include "../inc/webserv.hpp"
#include "../inc/config.hpp"
#include "../inc/error.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <ctime>

std::vector<server>	webserv::servers;

void	fd_sets::clear_sets()
{
	FD_ZERO(&this->read_fds);
	FD_ZERO(&this->read_fds_tmp);
	FD_ZERO(&this->write_fds);
	FD_ZERO(&this->write_fds_tmp);
}

std::string	webserv::get_corresponding_status(int status)
{
	if (status == 200)
		return ("HTTP/1.1 200 OK");
	else if (status == 400)
		return ("HTTP/1.1 400 Bad Request");
	else if (status == 403)
		return ("HTTP/1.1 403 Forbidden");
	else if (status == 404)
		return ("HTTP/1.1 404 Not Found");
	else if (status == 405)
		return ("HTTP/1.1 405 Method Not Allowed");
	else if (status == 413)
		return ("HTTP/1.1 413 Request Entity Too Large");
	else if (status == 414)
		return ("HTTP/1.1 414 Request Uri Too Long");
	else if (status == 500)
		return ("HTTP/1.1 500 Internal Server Error");
	else if (status == 501)
		return ("HTTP/1.1 501 Not Implemented");
	else if (status == 505)
		return ("HTTP/1.1 505 HTTP Version Not Supported");
	return ("");
}

void	webserv::check_timeout(fd_sets& set_fd)
{
	for (size_t i = 0; i < webserv::servers.size(); i++)
	{
		for (size_t j = 0; j < webserv::servers[i]._clients.size(); j++)
		{
			if (time(NULL) - webserv::servers[i]._clients[j]._connection_time >= TIMEOUT)
			{
				webserv::servers[i].close_connection(j, set_fd);
			}
		}
	}
}

void	webserv::serve_clients(fd_sets & set_fd, char** env)
{
	for (size_t index = 0; index < servers.size(); index++)
	{
		if (servers[index]._bound == false)
			continue;
		try
		{
			for (size_t j = 0; j < servers[index]._clients.size(); j++)
			{
				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.read_fds_tmp))
				{
					std::cout << "read request from fd " << servers[index]._clients[j].get_fd() << std::endl;
					servers[index]._clients[j].read_request(servers[index].get_config_index(), set_fd);
					servers[index]._clients[j]._connection_time = time(NULL);
				}

				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.write_fds_tmp))
				{
					if (servers[index]._clients[j]._request._chunked_body == true)
					{
						servers[index]._clients[j].unchunk_body_file(set_fd);
						continue;
					}

					if (servers[index]._clients[j]._cgi._cgi_processing == true)
					{
						std::cout << "dkhel l cgi" << std::endl;
						servers[index]._clients[j]._cgi.run_cgi(servers[index]._clients[j], env);
						if (servers[index]._clients[j]._cgi._cgi_processing == false)
						{
							servers[index]._clients[j]._response._requested_file.open(servers[index]._clients[j]._cgi._outfile.c_str());
							servers[index]._clients[j]._response.send_cgi_headers(servers[index]._clients[j].get_fd(), servers[index]._clients[j]._response._requested_file);
							// here no need to set the status because the headers are already sent and only the body that will be processed in the send_response()
							servers[index]._clients[j].set_ready_for_receiving_value(true);
						}
					}
					else if (servers[index]._clients[j].get_ready_for_receiving_value() == false)
					{
						std::cout << "handling request of client fd " << servers[index]._clients[j].get_fd() << std::endl;
						servers[index].handle_request(j, set_fd, servers[index]._clients[j]._location_index);
					}
					if (servers[index]._clients[j].get_ready_for_receiving_value() == true)
					{
						std::cout << "sending response to client with fd " << servers[index]._clients[j].get_fd() << std::endl;
						servers[index]._clients[j]._response.send_response(servers[index]._clients[j].get_fd(), get_corresponding_status(200),server::_config[servers[index].get_config_index()], servers[index]._clients[j]._connection_time);
						if (servers[index]._clients[j]._response._bytes_sent >= servers[index]._clients[j]._response._content_length)
						{
							std::cout << "all chunks are sent to fd " << servers[index]._clients[j].get_fd() << std::endl;
							FD_CLR(servers[index]._clients[j].get_fd(), &set_fd.write_fds);

							if (servers[index]._clients[j]._request._headers["Connection"] == "closed")
								throw error(CLOSE_CONNECTION, j);
		
							servers[index]._clients[j].clear_client();
						}
					}
				}
			}
		}
		catch (const error & e)
		{
			std::cout << "status catched in webserv::serve_clients: " << e._status << std::endl;

			if (e._status != CLOSE_CONNECTION) // because in case of CLOSE_CONNECTION i shouldn't send any error just close the connection
			{
				servers[index]._clients[e._client_index]._response._path_to_serve = server::_config[servers[index]._clients[e._client_index]._config_index].error_page_well_defined(e._status);
				if (servers[index]._clients[e._client_index]._response._path_to_serve.empty())
				{
					servers[index]._clients[e._client_index]._response.return_error(get_corresponding_status(e._status), servers[index]._clients[e._client_index].get_fd());
				}
				else
				{
					// server the headers and then set ready_for_receiving value = true so the file will be served by the same
					// function that serves the regular files

					servers[index]._clients[e._client_index]._response._status_code = e._status;
					servers[index]._clients[e._client_index].get_ready_for_receiving_value() = true;
				}
			}

			if (e._status == CLOSE_CONNECTION || e._status == -1 || e._status == 501 || e._status == 400 || e._status == 414 || e._status == 413)
				servers[index].close_connection(e._client_index, set_fd);
			else
			{
				servers[index]._clients[e._client_index].clear_client();
				FD_CLR(servers[index]._clients[e._client_index].get_fd(), &set_fd.write_fds);
			}
		}
	}
}

void	webserv::launch_server(char** env)
{
	socklen_t											client_addr_len;
	struct sockaddr_in									client_addr;
	int													client_sock;
	struct timeval										timeout;
	fd_sets												set_fd;
	int													nfds;
	int													select_rval;

	config::parse_mime_types("conf/mime.types");

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		servers.push_back(server(i));
		if (fcntl(servers.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
		{
			perror("fcntl F_SETFL");
			if (servers.back()._bound == true)
				close(servers.back().get_fd());
			servers.erase(servers.end() - 1);
		}
	}

	set_fd.clear_sets();
	nfds = 0;

	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i]._bound == true)
		{
			std::cout << "index of bound server = " << i << std::endl;
			nfds = servers[i].get_fd();
			FD_SET(servers[i].get_fd(), &set_fd.read_fds);
		}
	}

	client_addr_len = sizeof(struct sockaddr_in);
	memset(&client_addr, 0, client_addr_len);

	std::cout << "server size() = " << servers.size() << std::endl;
	std::cout << "number of bound addresses = " << server::_bound_addresses.size() << std::endl;


	while (true)
	{
		std::cout << "Waiting for connections...." << std::endl;

		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 0;
		set_fd.read_fds_tmp = set_fd.read_fds;
		set_fd.write_fds_tmp = set_fd.write_fds;

		select_rval = select(nfds + 1, &set_fd.read_fds_tmp, &set_fd.write_fds_tmp, NULL, &timeout);
		if (select_rval == -1)
		{
			perror("Error in select");
			for (size_t i = 0; i < servers.size(); i++)
			{
				if (servers[i]._bound == true)
					close(servers[i].get_fd());
			}
			throw 1;
		}

		webserv::check_timeout(set_fd);

		if (select_rval == 0)
			continue;

		for (size_t i = 0; i < servers.size(); i++)
		{
			if (servers[i]._bound == false)
				continue;

			if (FD_ISSET(servers[i].get_fd(), &set_fd.read_fds_tmp))
			{
				for (int iteration = 0; (client_sock = accept(servers[i].get_fd(), (struct sockaddr *)&client_addr, &client_addr_len)) != -1; iteration++)
				{
					if (client_sock == -1)
						perror("Error accepting connection");
					else
					{
						std::cout << "Connection accepted !!!\nClient number " << client_sock << std::endl;

						servers[i]._clients.push_back(client(client_sock, servers[i].get_config_index(), iteration));
						if (fcntl(servers[i]._clients.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
						{
							perror("fcntl F_SETFL");
							close(servers[i]._clients.back().get_fd());
							servers[i]._clients.remove_from_end(1);
						}
						else
						{
							if (client_sock > nfds)
							{
								nfds = client_sock;
							}
							FD_SET(servers[i]._clients.back().get_fd(), &set_fd.read_fds);
						}
					}
				}
			}
		}
		webserv::serve_clients(set_fd, env);
	}
}
