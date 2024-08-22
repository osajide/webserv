#pragma once

#include <vector>
#include "request.hpp"

class cgi
{
	public:
		cgi();
		~cgi();

		bool						_cgi_processing;
		char**						_env;
		char**						_args;
		bool						_first_time;
		std::string					_outfile;

		void		set_env_variables(request, char**);
		void		set_args(std::string path);
		void		run_cgi(request client_req, std::string path_to_serve, char** environ);

	private:
		std::string	get_random_file_name();
};