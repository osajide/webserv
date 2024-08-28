#pragma once

#include "request.hpp"
#include "response.hpp"
#include "cgi.hpp"
#include "fd_sets.hpp"
#include <fstream>
// #include <sys/_types/_timeval.h>

#define BUFFER_SIZE 500
#define CLOSE_CONNECTION 10

class client
{
	public:
		client(int client_sock, int conf_index, int index);
		client(client const &);
		client & operator=(const client &);

		int			get_fd();
		void		read_request(int config_index, fd_sets&);
		void		fill_request_object();
		void		set_ready_for_receiving_value(bool value);
		bool&		get_ready_for_receiving_value();

		time_t			_connection_time;
		int				_index;
		request			_request;
		response		_response;
		cgi				_cgi;

		int				_config_index;
		int				_location_index;

		int			dir_has_index_files();
		void		handle_delete_directory_request(fd_sets&);
		void		does_location_has_redirection();

		void		read_body_based_on_content_length(fd_sets& set_fd);
		void		read_chunked_body(fd_sets& set_fd);
		void		unchunk_body_file(fd_sets& set_fd);


		void		convert_numeric_values();

		void		clear_client();

	private:
		client();

		int				_fd;
		bool			_ready_for_receiving;
		bool			_read_body;
		size_t			_max_body_size;
		size_t			_bytes_read;
		std::fstream	_body_file;
		std::fstream	_unchunked_body_file;
};