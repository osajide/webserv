#include "../inc/cgi.hpp"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>

cgi::cgi() : _cgi_processing(false), _env_size(0), _first_time(true), _outfile("")
{}

void	cgi::set_env_variables(request client_req, char** environ)
{
	std::vector<std::string>	temp;

	temp.push_back("REQUEST_METHOD=" + client_req.get_method());
	this->_env_size = temp.size();

	this->_env = new char*[temp.size() + 1];
	for (size_t i = 0; i < temp.size(); i++)
	{
		this->_env[i] = new char[temp[i].size()];
		std::strcpy(this->_env[i], temp[i].c_str());
	}
	this->_env[temp.size() + 1] = NULL;
}

void	cgi::set_args(std::string path)
{
	this->_args = new char*[2];

	this->_args[0] = const_cast<char *>(path.c_str());
	this->_args[1] = NULL;
}

std::string	cgi::get_random_file_name()
{
	DIR*			directory;
	struct dirent*	entry;
	std::string 	file_name;

	file_name = "tmp";
	while (true)
	{
		directory = opendir("/tmp");
		while ((entry = readdir(directory)) != NULL)
		{
			if (file_name == entry->d_name)
			{
				file_name += '_';
				closedir(directory);
				break;
			}
		}
		if (entry == NULL)
			break;
	}
	return (file_name);
}

void	cgi::run_cgi(client & cl, char** environ)
{
	int				fd[2];
	int				pid;
	int				status;
	unsigned char*	st;

	if (this->_first_time == true)
	{
		this->set_args(cl._response._path_to_serve);
		this->set_env_variables(cl._request, environ);
		this->_outfile = this->get_random_file_name();
	
		fd[1] = open(this->_outfile.c_str(), O_CREAT, 0644);
		if (fd[1] == -1)
			throw 500;
		// if (client_req.get_method() == "POST")
		// {
		// 	fd[0] = open();
		// }

		pid = fork();
		if (pid == -1)
		{
			close(fd[1]);
			throw 500;
		}
		if (pid == 0)
		{
			if (dup2(fd[1], STDOUT_FILENO) == -1)
			{
				close(fd[1]);
				exit(EXIT_FAILURE); // then i catch the exit status and throw 500
			}
			close(fd[1]);

			execve(cl._response._path_to_serve.c_str(), this->_args, this->_env);
			close(fd[1]);
			exit(EXIT_FAILURE);
		}
	}
	else if (this->_first_time = false)
	{
		pid_t	wpid = waitpid(pid, &status, WNOHANG);
		if (wpid == -1)
		{
			close(fd[1]);
			throw 500;
		}
		if (wpid == 0)
			return ;
		if (wpid == pid)
		{
			// child finished
			st = (unsigned char *)&status;
			if (st[1] != 0)
			{
				close(fd[1]);
				throw 500;
			}
			else
			{
				// child finished with success
				close(fd[1]);
				cl._response.parse_cgi_response(this->_outfile);
			}
		}
	}
}

cgi::~cgi()
{
	int i;

	i = -1;
	while (this->_args[++i] != NULL)
		delete[] this->_args[i];

	delete[] this->_args;

	i = -1;
	while (this->_args[++i] != NULL)
		delete[] this->_args[i];

	delete[] this->_args;
}
