#pragma once

#include <fstream>
#include <vector>
#include <map>


typedef std::map<std::string, std::vector<std::string> > DirectiveMap;

typedef std::pair<std::string, DirectiveMap> LocationPair;

class config
{
	public:
		static std::vector<std::string>								_global_dictionary;
		static std::vector<std::string>								_location_dictionary;

		static void					set_dictionary();
		static void					parse_mime_types(const char* path_to_mime_types);

		config(std::fstream& file);
		config(config const &, std::string);

		void						check_validity_of_global_directives();
		void						check_validity_of_location_directives();

		std::vector<std::string>	fetch_directive_value(std::string key);
		int							directive_exists(std::string key);
		int							directive_inside_location_exists(int location_index, std::string key);
		std::vector<LocationPair>	get_locations();
		LocationPair				get_location_block(int location_index);
		std::vector<std::string>	fetch_location_directive_value(int location_index, std::string key);
		std::string					fetch_mime_type_value(std::string path);
		int							fetch_autoindex_value(int location_index);
		int							if_cgi_directive_exists(int location_index, std::string path);
		std::string					error_page_well_defined(int status);

	private:
		DirectiveMap												_directives;
		std::vector<LocationPair>									_locations;
		static std::vector<std::pair<std::string, std::string> >	_mime_types;
};

enum autoindx
{
	ON = 1,
	OFF
};