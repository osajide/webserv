#include "../inc/webserv.hpp"


int main(int ac, char** av, char** env)
{
	if (ac > 2)
	{
		std::cerr << "Invalid number of arguments" << std::endl;
		return (0);
	}

	std::string path_to_conf;

	try
	{
		if (ac == 1)
			path_to_conf = "../conf/sample.conf";
		else
			path_to_conf = av[1];

		server::parse_config(path_to_conf.c_str());
		webserv::launch_server(env);
	}
	catch (int status)
	{
		std::cout << "********* status catched = " << status << std::endl;
		return (0);
	}
	catch (std::string & msg)
	{
		std::cout << "--------- msg catched = " << msg << std::endl;
		return (0);
	}
}
