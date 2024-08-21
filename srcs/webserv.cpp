#include "../inc/webserv.hpp"
#include "../inc/server.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void	fd_sets::clear_sets()
{
	FD_ZERO(&this->read_fds);
	FD_ZERO(&this->read_fds_tmp);
	FD_ZERO(&this->write_fds);
	FD_ZERO(&this->write_fds_tmp);
}

void    webserv::launch_server(char** env)
{
    std::vector<server>									servers;
	socklen_t											client_addr_len;
	struct sockaddr_in									client_addr;
	int													client_sock;
	// struct timeval										connection_time;
	fd_sets												set_fd;
	int													nfds;

	config::parse_mime_types("conf/mime.types");

	for (size_t i = 0; i < server::_config.size(); i++) // temp because we should check if other server is trying to bind the same ip/port
	{
		servers.push_back(server(i));
		if (fcntl(servers.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
		{
        	perror("fcntl F_SETFL");
        	close(servers.back().get_fd());
        	servers.erase(servers.end() - 1);
    	}
	}

	set_fd.clear_sets();

	nfds = 0;

	for (size_t i = 0; i < servers.size(); i++)
	{
		FD_SET(servers[i].get_fd(), &set_fd.read_fds);
		// if (servers[i].get_fd() > nfds + 1)
			nfds = servers[i].get_fd() + 1;
	}

	client_addr_len = sizeof(struct sockaddr_in);
	memset(&client_addr, 0, client_addr_len);

	while (true)
	{
		std::cout << servers[0]._clients.size() << " clients available !!!!" << std::endl;

		for (size_t i = 0; i < servers[0]._clients.size(); i++)
		{
			if (FD_ISSET(servers[0]._clients[i].get_fd(), &set_fd.write_fds))
				std::cout << "fd " << servers[0]._clients[i].get_fd() << " is in write set" << std::endl;
			if (!FD_ISSET(servers[0]._clients[i].get_fd(), &set_fd.write_fds))
				std::cout << "fd " << servers[0]._clients[i].get_fd() << " is not in write set" << std::endl;
		}
		std::cout << "Waiting for connections...." << std::endl;

		set_fd.read_fds_tmp = set_fd.read_fds;
		set_fd.write_fds_tmp = set_fd.write_fds;

		if (select(nfds, &set_fd.read_fds_tmp, &set_fd.write_fds_tmp, NULL, NULL) == -1)
		{
			perror("Error in select");
			for (size_t i = 0; i < servers.size(); i++)
				close(servers[i].get_fd());
			throw 1;
		}

		for (size_t i = 0; i < servers.size(); i++)
		{
			if (FD_ISSET(servers[i].get_fd(), &set_fd.read_fds_tmp))
			{
				client_sock = accept(servers[i].get_fd(), (struct sockaddr *)&client_addr, &client_addr_len);
				if (client_sock == -1)
					perror("Error accepting connection");
				else
				{
					std::cout << "Connection accepted !!!\nClient number " << client_sock << std::endl;

					servers[i]._clients.push_back(client(client_sock, servers[i].get_config_index()));

					if (fcntl(servers[i]._clients.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
					{
        				perror("fcntl F_SETFL");
        				close(servers[i]._clients.back().get_fd());
        				servers[i]._clients.remove_from_end(1);
    				}
					else
					{
						nfds = client_sock + 1;
						FD_SET(servers[i]._clients.back().get_fd(), &set_fd.read_fds);
					}
				}
			}
		}

		for (size_t index = 0; index < servers.size(); index++)
		{
			for (size_t j = 0; j < servers[index]._clients.size(); j++)
			{
				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.read_fds_tmp))
				{
					try
					{
						servers[index]._clients[j].read_request(servers[index].get_config_index(), set_fd);
					}
					catch (int status)
					{
						std::cout << "status catched from read function = " << status << std::endl;

						if (status == -1)
							servers[index].close_connection(j, set_fd);

						else if (status >= 300 && status <= 308)
						{
							servers[index]._clients[j]._response.redirect(servers[index]._clients[j].get_fd(), status, servers[index]._clients[j]._response._redirection_path);
							servers[index]._clients[j].clear_client();
							FD_CLR(servers[index]._clients[j].get_fd(), &set_fd.write_fds);
						}
						else
						{
							servers[index]._clients[j]._response.return_error(status, servers[index]._clients[j].get_fd());
							servers[index].close_connection(j, set_fd);
						}
						continue;
					}
				}

				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.write_fds_tmp))
				{
					try
					{
						if (servers[index]._clients[j]._cgi._cgi_processing = true)
						{
							servers[index]._clients[j]._cgi.run_cgi(servers[index]._clients[j], env);
							
							if (servers[index]._clients[j]._cgi._cgi_processing == false)
							{
								servers[index]._clients[j]._response._requested_file.open(servers[index]._clients[j]._cgi._outfile);
								servers[index]._clients[j].set_ready_for_receiving_value(true);
							}
						}
						if (servers[index]._clients[j].get_ready_for_receiving_value() == false)
						{
							std::cout << "handling request of client fd " << servers[index]._clients[j].get_fd() << std::endl;
							servers[index].handle_request(j, set_fd, servers[index]._clients[j]._location_index, env);
						}

						if (servers[index]._clients[j].get_ready_for_receiving_value() == true)
						{
							std::cout << "sending response to client with fd " << servers[index]._clients[j].get_fd() << std::endl;
							servers[index]._clients[j]._response.send_response(servers[index]._clients[j].get_fd(), server::_config[servers[index].get_config_index()]);

							if (servers[index]._clients[j]._response._bytes_sent >= servers[index]._clients[j]._response._content_length)
							{
								std::cout << "all chunks are sent to fd " << servers[index]._clients[j].get_fd() << std::endl;
								servers[index]._clients[j].clear_client();
								FD_CLR(servers[index]._clients[j].get_fd(), &set_fd.write_fds);
							}
						}
					}
					catch (int status)
					{
						servers[index]._clients[j]._response.return_error(status, servers[index]._clients[j].get_fd());
					}
				}
			}
		}
	}
}
