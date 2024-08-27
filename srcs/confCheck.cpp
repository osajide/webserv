#include "../inc/webserv.hpp"
#include <ostream>
#include <string>
#include <sys/_types/_size_t.h>

std::string	trim_whitespace(std::string str)
{
	size_t start = str.find_first_not_of(" \t\r\v\n");
	if (start == str.find_last_not_of(" \t\r\v\n") && start == std::string::npos)
		return "";

	// std::cout << str << " " << start << " " << str.find_last_not_of(" \t\r\v\n") << std::endl;
	if (start != std::string::npos)
		str= str.std::string::substr(start, str.find_last_not_of(" \t\r\v\n") - str.length());
	return str;
}

int	is_valid_IP(std::string ip) {
      		std::cout << ip << std::endl;
	return (1);
}

void run_check(char *conf) {
	std::ifstream myConf(conf);
	std::string content;
	int braces = 0;
	char *lim;

	if (myConf.is_open()) {
		while (std::getline(myConf, content)) {
			content = trim_whitespace(content);
			if (content == "{")
				braces++;
			else if (content == "}")
				braces--;
			if ((content == "{" || content == "}") || (!braces && !content.length()))
				continue;
			if ((content == "server" && braces) || (!braces && content != "server"))
				throw("Invalid config structure!");

			lim = strtok(const_cast<char *>(content.c_str()), " ");
			if (content != "server" && content != "{" && content != "}") {
				if (std::strcmp(lim, "location") && (content.find(';') == std::string::npos || content.find(';') != content.length() - 1)){
					throw("Invalid config structure!");}
			}
			while (lim != NULL) {
				lim = strtok(NULL, " ");
			}
		}
		if (braces)
			throw("Invalid config structure!");

	} else
    	throw("Couldn't open the config file!");
}