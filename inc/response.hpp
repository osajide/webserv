#pragma once

#include "config.hpp"
#include <fstream>
#include <string>
#include <iostream>

class response
{
    public:
        response();
        response(response const &);
        response &  operator=(response const &);

        void            set_status_line(std::string);
        void            set_content_length(std::ifstream&);
        void            set_location(std::string);
        void            set_body(std::string);

        // std::string get_status_line();
        // std::string get_content_type();
        long long       get_content_length();
        // std::string get_location();
        // std::string get_body();
        std::string     get_chunk(std::ifstream&);

        void            send_headers(int target_fd);

        void            return_error(int status, int target_fd);
        void            clear_response();


        void            set_path_to_serve(std::string path);
        std::string     get_path_to_serve();

        void            send_response(int fd, config serverConf);
        void            return_index_file(std::string path);

        /*=============== ATTRIBUTES ===============*/

        long long       _bytes_sent;
        std::string     _status_line;
        long long       _content_length;
        std::string     _content_type;
        std::string     _location;
        std::string     _body;
        std::string     _headers;
        std::string     _chunk;
        long            _bytes_written;
        std::string     _unsent_part;
        std::ifstream   _requested_file;
        std::string     _path_to_serve;

};