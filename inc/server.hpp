#pragma once

#include "config.hpp"
#include "client.hpp"
#include "types.hpp"
#include <string>
// #include <sys/_types/_fd_def.h>
#include <vector>
#include <netinet/in.h>
#include <iostream>
#include "webserv.hpp"

class server
{
    public:
        static  void	parse_config(char* PathToConfig);
		static	int		if_ip_port_already_bound(std::string ip, std::string port);

        static std::vector<config>									_config;
        static std::vector<std::pair<std::string, std::string> >	_bound_addresses;
		
		server(int _conf_index);


		void		init_socket();
		int			get_fd();
		int			get_config_index();
		// void		handle_request(int client_index, fd_set& write_fds, int location_index);
		void		handle_request(int client_index, fd_sets &, int location_index);
		// void		send_response(int client_index, fd_set& write_fds);
		void		check_if_method_allowed_in_location(int client_index, int location_index);
		std::string	check_availability_of_requested_resource(int client_index, int location_index);
		int         check_resource_type(std::string path);
		// void		close_connection(int client_index, fd_set & read_fds, fd_set & write_fds);
		void		close_connection(int client_index, fd_sets &);

		myList				_clients;
    private:
        server();
        int					_fd;
        std::string			_ip;
        std::string			_port;
		int					_conf_index;
		sockaddr_in			_addr;
};

enum    path_check
{
    DIRECTORY = 1,
    REG_FILE
};