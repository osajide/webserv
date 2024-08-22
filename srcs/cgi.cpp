#include "../inc/cgi.hpp"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>

cgi::cgi() : _cgi_processing(false), _env(NULL), _args(NULL), _first_time(true), _outfile("")
{}

void	cgi::set_env_variables(request client_req, char** environ)
{
	std::vector<std::string>	temp;

	temp.push_back("REQUEST_METHOD=" + client_req.get_method());

	size_t	index;
	size_t	j;
	size_t	k;

	index = 0;
	while (environ[index] != NULL)
		index++;

	this->_env = new char*[index + temp.size() + 1];

	j = 0;
	while (j < index)
	{
		this->_env[j] = new char[std::strlen(environ[j]) + 1];
		std::strcpy(this->_env[j], environ[j]);
		j++;
	}
	k = 0;
	while (k < temp.size())
	{
		this->_env[j] = new char[temp[k].size() + 1];
		std::strcpy(this->_env[j], temp[k].c_str());
		j++;
		k++;
	}

	this->_env[temp.size() + index] = NULL;
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
		{
			closedir(directory);
			break;
		}
	}
	return (file_name);
}

void	cgi::run_cgi(request client_req, std::string path_to_serve, char** environ)
{
	int				fd[2];
	int				pid;
	int				status;
	unsigned char*	st;

	status = -1;
	pid = -1;
	fd[0] = -1;
	fd[1] = -1;

	if (this->_first_time == true)
	{
		this->set_args(path_to_serve);
		this->set_env_variables(client_req, environ);
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

			execve(path_to_serve.c_str(), this->_args, this->_env);
			close(fd[1]);
			exit(EXIT_FAILURE);
		}
		else
		{
			this->_first_time = false;
		}
	}
	else if (this->_first_time == false)
	{
		pid_t	wpid = waitpid(pid, &status, WNOHANG);
		std::cout << "wpid = " << wpid << std::endl;
		std::cout << "pid = " << pid << std::endl;
		if (wpid == -1)
		{
			close(fd[1]);
			throw 500;
		}
		if (wpid == 0)
			return ;
		if (wpid > 0)
		{
			st = (unsigned char *)&status;
			(void)st;
			// if (st[0] != 0 || st[1] != 0)
			// {
			// 	std::cout << "st[0] = '" << (int)st[0] << "'" << std::endl;
			// 	std::cout << "st[1] = '" << (int)st[1] << "'" << std::endl;
			// 	std::cout << "dkhel l hadi" << std::endl;
			// 	exit(0);
			// 	close(fd[1]);
			// 	throw 500;
			// }
			// else
			// {
				// child finished with success
				std::cout << "child finished with success" << std::endl;
				close(fd[1]);
				this->_cgi_processing = false;
			// }
		}
	}
}

cgi::~cgi()
{
	int i; 

	if (this->_args != NULL)
	{
		i = -1;
		while (this->_args[++i] != NULL)
		{
			delete[] this->_args[i];
		}

		delete[] this->_args;
		// this->_args = NULL;
	}

	if (this->_env != NULL)
	{
		i = -1;
		while (this->_env[++i] != NULL)
		{
			delete[] this->_env[i];
		}

		delete[] this->_env;
		// this->_env = NULL
	}
}
