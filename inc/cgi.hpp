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
		int							_env_size;
		bool						_first_time;

		void	set_env_variables(request);
		void	run_cgi(request);
};