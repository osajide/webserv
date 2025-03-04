# 🖥️ C++ Http server

## 🌟 Overview

This project is a lightweight web server written in C++ that efficiently handles HTTP requests without using threads. Instead, it utilizes multiplexing to manage multiple connections simultaneously. The server also supports Common Gateway Interface (CGI) for executing scripts and generating dynamic content.

## ✨ Features

- 🚀 Efficient handling of HTTP requests using multiplexing.
- 📄 Support for GET, POST and DELETE methods.
- ⚙️ CGI support for dynamic content generation.

## 📋 Requirements

- POSIX-compliant operating system (e.g., Linux, macOS)
- GNU Make

## 📦 Installation

   ```bash
   git clone https://github.com/osajide/webserv.git
   cd webserv
   ```
## 🔧 Configuration

You can configure the server by making your own configuration file inside the `conf` directory. There is also a file `conf/default.conf` that contains an example to start with. The format of the configuration file is inspired by Nginx (not all directives).

```nginx
server
{
	listen 127.0.0.1:8183 127.0.0.1:65535 127.0.0.1:8181; # This will listen on three ports
	# root /path/to/root/directory;
	root $REPO_ROOT/content/; # REPO_ROOT holds the value of the root of the repository

	# client max body size in bytes
	client_max_body_size 200000000;

	location /
	{
		index index.html;

		# put extensions that you want your cgi to accept
		cgi py jsx php;

		allowed_methods GET POST DELETE;

		# upload_dir /path/where/to/upload;
		upload_dir $REPO_ROOT/content/uploads;

		autoindex on; # Enable directory listing
	}
	location /cgi-bin
	{
		index forms/cgi.html;
		cgi py jsx php;

		allowed_methods GET POST DELETE;
		upload_dir $REPO_ROOT/content/uploads;
		autoindex on;
	}
}

```
The `mime.types` file is used to define the MIME types supported by the server. To add a new MIME type, simply add it to this file `conf/mime.types`. For example:

```nginx
# mime.types

text/html                            html;
text/css                             css;
image/png                            png;
...
# Add new MIME types here
video/mp4                            mp4;
image/webp                           webp;
```

## 🚀 Launch the server

1. Build the project using the `make` command:

   ```bash
   make
   ```
3. Run the server with the executable and path to the configuration file:

   ```bash
   ./webserv /path/to/your/config
   ```
   You can also use:
   ```bash
   ./webserv
   ```
   And the default configuration will be used.
   The server will start listening on the port specified in the configuration file.