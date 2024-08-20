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
		int							_env_size;
		bool						_first_time;
		std::string					_outfile;

		void		set_env_variables(request);
		void		set_args(std::string path);
		void		run_cgi(request, std::string path);

	private:
		std::string	get_random_file_name();
};