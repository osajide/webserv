#include "../inc/webserv.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ostream>
#include <cstring>
#include <sstream>

std::string trim_whitespace(std::string str) {
	size_t start = str.find_first_not_of(" \t\r\v\n");
	if (start == str.find_last_not_of(" \t\r\v\n") && start == std::string::npos)
		return "";

	if (start != std::string::npos)
		str = str.std::string::substr(start, str.find_last_not_of(" \t\r\v\n") -
												str.length());
	return str;
}

int	count_char(std::string str, char c) {
	int	count = 0;
	
	for (int i = 0; str.find(c, i) != std::string::npos; i++) {
		i = str.find(c, i);
		count++;
	}
	return count;
}

void	check_nums(std::string str, char set) {
	std::stringstream stream(str);
	std::string lim;
	int i = -1;
	char *endp;

	while (std::getline(stream, lim, set)) {
		if (count_char(lim, '.'))
			check_nums(lim, '.');
		else {
			i = std::strtod(const_cast<char *>(lim.c_str()), &endp);
			if (i < 0 || (set != ':' && (endp[0] != 0 || i > 255)) || (set == ':' && i > 65535))
				throw("Invalid IP address!");
		}
	}
}

void is_valid_IP(std::string ip) {
	if (count_char(ip, '.') != 3 || count_char(ip, ':') != 1)
		throw("Invalid IP address!");
	check_nums(ip, ':');
}

void run_check(char *conf) {
	std::ifstream myConf(conf);
	std::string content;
	int braces = 0;
	char *lim;
	int checkIp;

	if (myConf.is_open()) {
		while (std::getline(myConf, content)) {
			checkIp = 0;
			content = trim_whitespace(content);
			if (content.find('\t') != std::string::npos)
				throw("Invalid config structure!");
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
				if (std::strcmp(lim, "location") &&
					(content.find(';') == std::string::npos ||
					content.find(';') != content.length() - 1 ||
					content.find('{') != std::string::npos ||
					content.find('}') != std::string::npos)) {
					throw("Invalid config structure!");
				}
			}
			if (!std::strcmp(lim, "listen"))
				checkIp = 1;
			int iter = 1;
			while (lim != NULL) {
				lim = strtok(NULL, " ");
				if (checkIp && lim)
					is_valid_IP(lim);
				iter++;
			}
			if (content != "server" && iter < 3)
					throw("Missing value!");
		}
		if (braces)
			throw("Invalid config structure!");

	} else
		throw("Couldn't open the config file!");
}