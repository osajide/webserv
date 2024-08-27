#pragma once

#include "fd_sets.hpp"
#include "server.hpp"

#define TIMEOUT 60

class   webserv
{
	public:
		static std::vector<server>	servers;
		static void					launch_server(char** env);
		static void					serve_clients(fd_sets & set_fd, char** env);
		static void					check_timeout(fd_sets & set_fd);
		static std::string			get_corresponding_status(int status);
};
