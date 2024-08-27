#include "../inc/webserv.hpp"
#include <algorithm>
#include <cstdlib>
#include <ostream>
#include <string>

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

void	check_nums(std::string str, const char *set) {
	char *lim;
	int i = -1;
	char *endp;

	lim = strtok(const_cast<char *>(str.c_str()), set);
	while (lim != NULL) {
		if (count_char(lim, '.'))
			check_nums(lim, ".");
		else {
			i = std::strtod(lim, &endp);
			if (endp[0] != 0 || i < 0 || i > 255)
				throw("Invalid IP address!");
		}
		std::cout << lim << std::endl;
		lim = strtok(NULL, set);
	}
}

int is_valid_IP(std::string ip) {
	if (count_char(ip, '.') != 3 || count_char(ip, ':') != 1)
		throw("Invalid IP address!");
	check_nums(ip, ":");
	return (1);
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
			while (lim != NULL) {
				lim = strtok(NULL, " ");
				if (checkIp && lim)
					is_valid_IP(lim);
			}
		}
		if (braces)
			throw("Invalid config structure!");

	} else
		throw("Couldn't open the config file!");
}