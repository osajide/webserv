#include "../inc/webserv.hpp"
#include "../inc/server.hpp"
#include <iostream>


int main(int ac, char** av)
{
    if (ac != 2)
    {
        std::cerr << "Invalid number of arguments" << std::endl;
        return (0);
    }

    try
    {
        server::parse_config(av[1]);
        webserv::launch_server();
    }
    catch (int status)
    {
        std::cout << "********* status catched = " << status << std::endl;
        return (0);
    }
}
/*
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

void	remove_uri(std::string path)
{
	DIR*			directory;
	struct dirent*	entry;

	directory = opendir(path.c_str());

	while ((entry = readdir(directory)) != NULL)
	{
		std::string	entry_str;
		struct stat entry_type;

		entry_str = path;
		entry_str += "/" + entry->d_name;
		std::cout << "entry_str = '" << entry_str << "'" << std::endl;

		stat(entry_str.c_str(), &entry_type);

		if (S_ISREG(entry_type.st_mode))
		{
			if (std::remove(uri.c_str()) != 0)
				this->return_error(403, fd);}
		}
		if (S_ISDIR(entry_type.st_mode))
		{
			remove_uri(entry->d_name);
		}
	}
}

int main(int ac, char **av)
{
	(void)ac;

	remove_uri(av[1]);
}

*/