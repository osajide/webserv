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
        void        read_request(int config_index, fd_sets&);
        void        fill_request_object();
        void	    set_ready_for_receiving_value(bool value);
		bool	    get_ready_for_receiving_value();

        request     _request;
        response    _response;

        int         _config_index;
        int         _location_index;

        int         if_cgi_directive_exists();
        int         dir_has_index_files();
        void        handle_delete_request();

    private:
        client();

        int         _fd;
        bool		_ready_for_receiving;
        // std::string _path_to_serve;
};