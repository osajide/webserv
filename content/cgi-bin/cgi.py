#!/usr/bin/env python3

import os
import urllib.parse
import sys

meth = os.environ['REQUEST_METHOD']
data = ''

if (meth == 'POST'):
	while True:
		line = sys.stdin.readline()
		if not line:
			break
		data += line
else:
	data = os.environ['QUERY_STRING']

data = urllib.parse.unquote(data.replace('+', ' ')).split('&')

body = f"""
	<!DOCTYPE html>
	<html lang="en">

	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>CGI</title>
		<link rel="stylesheet" href="/cgi-bin/forms/styles/style.css" />
	</head>

	<body>
		<div class="response">
			<h1>Methode: {meth}</h1>
			<h2>{data[2].replace('=', ': ')}</h2>
			<h2>{data[3].replace('=', ': ')}</h2>
			<h2>{data[4].replace('=', ': ')}</h2>
		</div>
	</body>

	</html>
"""

print(f"HTTP/1.1 200 OK\r\nContent-Length: {len(body)}\r\nContent-Type: text/html\r\n\r\n{body}", end="")