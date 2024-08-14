#pragma once

#include "request.hpp"
// #include <sys/_types/_fd_def.h>
#include <sys/select.h>
#include <string>
#include <iostream>
#include "response.hpp"
#include "webserv.hpp"


#define BUFFER_SIZE 4999

class client
{
    public:
        client(int client_sock, int conf_index);

        int         get_fd();
        void        read_request(int config_index, fd_sets& , int& location_index);
        void        fill_request_object();
        void	    set_ready_for_receiving_value(bool value);
		bool	    get_ready_for_receiving_value();
        // void        set_path_to_serve(std::string path);
        // std::string get_path_to_serve();

        request     _request;
        response    _response;

        int         _config_index;

    private:
        client();

        int         _fd;
        bool		_ready_for_receiving;
        // std::string _path_to_serve;
};