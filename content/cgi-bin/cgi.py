#!/usr/bin/env python3

import os
import urllib.parse
import sys
import re

meth = os.environ['REQUEST_METHOD']
data = ''
disp = ''
if "CONTENT_TYPE" in os.environ:
	contentType = os.environ['CONTENT_TYPE']

def parse_filename(content):
    filename_pattern = r'filename="([^"]+)"'
    match = re.search(filename_pattern, content, re.IGNORECASE)
    return match.group(1).strip() if match else ""


def formatData():
	for j, elm in enumerate(data):
		header, body = elm.split(b'\r\n\r\n', 1)
		filename = parse_filename(header.decode())
		if not filename:
			header = header.decode("utf-8")
			ret = ''.join(header.split('Content-Disposition: form-data; '))
			ret = ret.replace('"', ': ')
			ret = ret.replace('name=: ', '', 1)
			data[j] = f"{ret}{body.decode('utf-8')}".strip()
		else:
			with open(filename, "wb") as f:
				f.write(body)
			data[j] = ''
	

def addH2():
	tmp = ''
	for index, elem in enumerate(data):
		if (index > 1 and len(elem) > elem.find('=') + 1):
			tmp = tmp + f'<h2>{elem}</h2>'
	return tmp

if (meth == 'POST'):
	data = sys.stdin.buffer.read()

	contentType = contentType.split(';')
	if (contentType[0] == "multipart/form-data") :
		tmp = '--' + contentType[1].split('=')[1]
		data = data.split(tmp.encode())
		data.pop(0)
		data.pop()
		formatData()
	else:
		data = urllib.parse.unquote(data.replace('+', ' ')).split('&')
	
else:
	data = os.environ['QUERY_STRING']
	data = urllib.parse.unquote(data.replace('+', ' ')).split('&')
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

print(f"HTTP/1.1 200 OK\r\nContent-Length: {len(body)}\r\nContent-Type: text/html\r\n\r\n{body}", end="")
