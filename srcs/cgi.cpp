#include "../inc/cgi.hpp"
#include <cstring>

cgi::cgi() : _cgi_processing(false), _env_size(0), _first_time(true)
{}

void	cgi::set_env_variables(request client_req)
{
	std::vector<std::string>	temp;

	temp.push_back("REQUEST_METHOD=" + client_req.get_method());
	this->_env_size = temp.size();

	this->_env = new char*[temp.size()];
	for (size_t i = 0; i < temp.size(); i++)
	{
		this->_env[i] = new char[temp[i].size()];
		std::strcpy(this->_env[i], temp[i].c_str());
	}
}

void	cgi::run_cgi(request client_req)
{
	int	fd[2];

	if (this->_first_time == true)
	{
		// fd[0] = open();
	}
	else if (this->_first_time = false)
	{

	}
}

cgi::~cgi()
{
	for (size_t i = 0; i < this->_env_size; i++)
	{
		delete[] this->_env[i];
	}
	delete[] this->_env;
}
