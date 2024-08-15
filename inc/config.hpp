#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include <map>


typedef std::map<std::string, std::vector<std::string> > DirectiveMap;

typedef std::pair<std::string, DirectiveMap> LocationPair;

class config
{
    public:
        config(std::fstream& file);
		std::vector<std::string>	fetch_directive_value(std::string key);
        int	                        directive_exists(std::string key);
        int                         directive_inside_location_exists(int location_index, std::string key);
		std::vector<LocationPair>   get_locations();
		LocationPair				get_location_block(int location_index);
		std::vector<std::string>	fetch_location_directive_value(int location_index, std::string key);
		std::string					fetch_mime_type_value(std::string path);
		static void					parse_mime_types(const char* path_to_mime_types);

    private:
        DirectiveMap                			                    _directives;
        std::vector<LocationPair>   			                    _locations;
        std::vector<std::string>                                    _allowed_directives;
        static std::vector<std::pair<std::string, std::string> >    _mime_types;
};