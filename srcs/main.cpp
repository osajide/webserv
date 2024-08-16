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
