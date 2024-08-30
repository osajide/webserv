#!/usr/bin/env python3

import os
import urllib.parse
import sys

meth = os.environ['REQUEST_METHOD']
data = ''
disp = ''
# contentType = os.environ['CONTENT_TYPE']
# def formatData (elm):
# 	if (elm.indexOf('filename') < 0):
# 		ret = elm.split('Content-Disposition: form-data; ').join('')
# 		ret = ret.substr(6).replace('"', ': ')
# 		return ret
# 	else:
# 		return ''

def addH2():
    tmp = ''
    for index, elem in enumerate(data):
        if (index > 1 and len(elem) > elem.index('=') + 1):
        	tmp = tmp + f'<h2>{elem}</h2>'
    return tmp

# if (meth == 'POST'):
# 	while True:
# 		line = sys.stdin.readline()
# 		if not line:
# 			break
# 		data += line
# 	contentType = contentType.split(';')
# 	if (contentType[0] == "multipart/form-data") :
# 		data = data.split('--' + contentType[1].split('=')[1])
# 		data.shift()
# 		data.pop()
# 		data = map(formatData, data)
	
# else:
data = os.environ['QUERY_STRING']
data = urllib.parse.unquote(data.replace('+', ' ')).split('&')
# disp = data
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