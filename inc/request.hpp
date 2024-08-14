#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "config.hpp"

class request
{
    public:
		void        set_raw_request(char* buffer);
		std::string	get_rawRequest();
		
        void    	set_request_line(std::string    request_line);
        void    	set_header(std::string key, std::string value);
        
		std::string get_method();
        std::string get_target();
        std::string get_http_version();
		void		is_well_formed(config &);
		int			does_uri_match_location(std::vector<LocationPair> locations, std::string uri_target);
        std::string fetch_header_value(std::string key);


		void		clear_request();

    private:
    	std::string							_rawRequest;
        std::string                         _method;
        std::string                         _target;
        std::string                         _http_version;
        std::map<std::string, std::string>  _headers;

};