#!/usr/bin/env python3

import os
import urllib.parse
import sys

meth = os.environ['REQUEST_METHOD']
data = ''
disp = ''
if "CONTENT_TYPE" in os.environ:
	contentType = os.environ['CONTENT_TYPE']

# def decode():


def formatData():
	for j, elm in enumerate(data):
		if (elm.find('filename') < 0):
			ret = ''.join(elm.split('Content-Disposition: form-data; '))
			ret = ret.replace('"', ': ')
			ret = ret.replace('name=: ', '', 1)
			data[j] = ret
		else:
			data[j] = ''
	

def addH2():
    tmp = ''
    for index, elem in enumerate(data):
        if (index > 1 and len(elem) > elem.find('=') + 1):
        	tmp = tmp + f'<h2>{elem}</h2>'
    return tmp

if (meth == 'POST'):
	while True:
		line = sys.stdin.readline()
		if not line:
			break
		data += line

	contentType = contentType.split(';')
	if (contentType[0] == "multipart/form-data") :
		data = data.split('--' + contentType[1].split('=')[1])
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
