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
	dir = 'uploads/'
	# exit (1)

data = ''
disp = ''
myFile = ''

if "CONTENT_TYPE" in os.environ:
	contentType = os.environ['CONTENT_TYPE']
else:
	exit (1)

def parse_filename(content):
    filename_pattern = r'filename="([^"]+)"'
    match = re.search(filename_pattern, content, re.IGNORECASE)
    return match.group(1).strip() if match else ""


def formatData():
	global myFile
	for j, elm in enumerate(data):
		header, body = elm.split(b'\r\n\r\n', 1)
		filename = parse_filename(header.decode())
		if not filename:
			exit (1)
		else:
			myFile = filename
			with open(f'{dir}{filename}', "wb") as f:
				f.write(body)

data = sys.stdin.buffer.read()

contentType = contentType.split(';')
if (contentType[0] == "multipart/form-data") :
	tmp = '--' + contentType[1].split('=')[1]
	data = data.split(tmp.encode())
	data.pop(0)
	data.pop()
	formatData()

disp = f'<h2>File: {myFile} uploaded</h2>'

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

print(f"HTTP/1.1 201 Created\r\nContent-Length: {len(body)}\r\nContent-Type: text/html\r\n\r\n{body}", end="")
