#include "../inc/request.hpp"
#include <sstream>

void    request::set_raw_request(char* buffer)
{
    this->_rawRequest += buffer;
}

std::string	request::get_rawRequest()
{
	return (this->_rawRequest);
}

void    request::set_request_line(std::string request_line)
{
    std::stringstream   ss(request_line);

    ss >> this->_method;
    ss >> this->_target;
    ss >> this->_http_version;
}

void    request::set_header(std::string key, std::string value)
{
    this->_headers[key] = value;
}

std::string	request::get_method()
{
	return (this->_method);
}

std::string	request::get_target()
{
	return (this->_target);
}

std::string	request::get_http_version()
{
	return (this->_http_version);
}
#include <unistd.h>
void	request::is_well_formed()
{
	if (this->_headers.find("Transfer-Encoding") != this->_headers.end()
		&&	this->_headers["Transfer-Encoding"] != "chunked")
	{
		throw 501; // Not Implemented
	}
	if (this->_method == "POST" && this->_headers.find("Transfer-Encoding") == this->_headers.end()
				&& this->_headers.find("Content-Length") == this->_headers.end())
	{
		throw 400; // Bad Request
	}
	if (this->_method == "POST" && this->_headers.find("Transfer-Encoding") != this->_headers.end()
				&& this->_headers.find("Content-Length") != this->_headers.end())
	{
		throw 400;
	}
	if (this->_target.length() > 2048)
	{
		throw 414; // Request Uri Too Long
	}
	// connection or host fields empty and check http version
	if (this->_headers.find("Connection") == this->_headers.end()
			|| (this->_headers["Connection"] != "keep-alive" && this->_headers["Connection"] != "closed"))
	{
		throw 400;
	}
	if (this->_headers.find("Host") == this->_headers.end() || this->_headers["Host"].empty())
	{
		throw 400;
	}
	if (this->_http_version == "undefined")
		sleep(5);
	if (this->_http_version != "HTTP/1.1")
	{
		throw 505; //HTTP Version Not Supported
	}

}

std::string remove_last_dir_from_path(std::string  path)
{
	int			i;
    std::string new_path;

	i = path.size() - 1;
	if (path[i] == '/')
		i--;

	if (i == -1)
		return ("");

	while (i >= 0)
	{
		if (path[i] == '/')
			break;
		i--;
	}
	new_path = path.substr(0, i);

    if (new_path.empty() && path[0] == '/')
	    new_path = path.substr(0, i + 1);

    return (new_path);
}

int	request::does_uri_match_location(std::vector<LocationPair> locations, std::string uri_target)
{
	if (!locations.empty())
    {
        if (uri_target.size() > 1 && uri_target.back() == '/') // check the size in case of request line like this "GET / HTTP/1.1"
		{
            uri_target.erase(uri_target.size() - 1, 1);
        }

	    while (!uri_target.empty())
	    {
	    	for (size_t i = 0; i < locations.size(); i++)
	    	{
	    		if (uri_target == locations[i].first)
	    		{
	    			return (i);
	    		}
	    	}
	    	uri_target = remove_last_dir_from_path(uri_target);
	    }
    }
	return (-1); // we will return -1 to use the default (root)
}

std::string	request::fetch_header_value(std::string key)
{
	if (this->_headers.find(key) != this->_headers.end())
		return (this->_headers[key]);
	return ("");
}
/*
// int             Request::hexToInt(const std::string& hexStr) 
int             hexToInt(const std::string& hexStr) 
{
    return std::stoi(hexStr, nullptr, 16);
}

// std::string     Request::convertChars(const std::string& path)
std::string     convertChars(const std::string& path)
{
    std::string str = "";
    std::string allowed_chars;
    std::string nums = "0123456789";
    std::string num = "";

    allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (int i = 0; i < path.size() ; i++)
    {
        num.clear();
        if (path[i] != '%' && allowed_chars.find(path[i]) != std::string::npos)
            str += path[i];
        else
        {
            if (path[++i])
            {
                while (path[i] && nums.find(path[i]) != std::string::npos)
                    num += path[i++];
                
                if (!num.empty())
                {
                    num = hexToInt(num); 
                    str += num;
                }
            }
        }
    }
    return (str);
}

// int      Request::notAllowedChar(const std::string& path)
int      notAllowedChar(const std::string& path)
{
    std::string allowed_chars;
    std::string converted_str;

    allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    
    converted_str = convertChars(path);
    for(int i = 0; i < converted_str.size(); i++)
    {
        if (allowed_chars.find(converted_str[i]) == std::string::npos)
            return (1);
    }
    return (0);
}*/

void	request::clear_request()
{
	this->_rawRequest.clear();
	this->_method.clear();
	this->_target.clear();
	this->_http_version.clear();
	this->_headers.clear();
}