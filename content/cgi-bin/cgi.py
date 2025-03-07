#!/usr/bin/env python3

import os
import urllib.parse
import sys
import re

if "REQUEST_METHOD" in os.environ:
	meth = os.environ['REQUEST_METHOD']
else:
	exit (1)

if "UPLOAD_DIR" in os.environ:
	dir = os.environ['UPLOAD_DIR']
else:
	exit (1)

data = ''
disp = ''
status = ["200", "OK"]

def addH2():
	tmp = ''
	for index, elem in enumerate(data):
		if (index > 1 and len(elem) > elem.find('=') + 1):
			tmp = tmp + f'<h2>{urllib.parse.unquote(elem).replace("+", " ").replace("=", ": ", 1)}</h2>'
	return tmp

if (meth == 'POST'):
	data = sys.stdin.buffer.read()
	data = data.decode("utf8").split('&')
else:
	if "QUERY_STRING" in os.environ:
		data = os.environ['QUERY_STRING']
		data = data.split('&')
	else:
		exit (1)
	if (meth == "DELETE"):
		myfl = ''
		for index, elem in enumerate(data):
			tmp = elem.split("=")
			if (tmp[0] == "fileName"):
				myfl = dir + tmp[1]
				if(not os.path.exists(myfl)):
					exit(1)
			else:
				exit(1)
		
		for index, elem in enumerate(data):
			tmp = elem.split("=")[1]
			try:
				myfl = dir + tmp
				os.remove(myfl)
				status = ["204", "No Content"]
			except Exception:
				exit(1)



disp = " ".join(status)
if (meth != "DELETE"):
	disp = addH2()

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
			{disp}
		</div>
	</body>

	</html>
"""

print(f"HTTP/1.1 {' '.join(status)}\r\nContent-Length: {len(body)}\r\nContent-Type: text/html\r\n\r\n{body}", end="")
