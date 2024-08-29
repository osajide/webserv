#!/usr/bin/env node

meth = process.env.REQUEST_METHOD
contentType = process.env.CONTENT_TYPE

const displayRes = (formD) => {
	body = `
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
				<h1>Methode: ${meth}</h1>
				${formD.map((a, index) => {
					if (index > 1 && a.length > a.indexOf('=') + 1)	
						return `<h2>${a?.replace('=', ': ')}</h2>`
				}).join('')}
			</div>
		</body>
	
		</html>
	`

	console.log(`HTTP/1.1 200 OK\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n\r\n${body}`)
}

if (meth == 'POST') {
	data = ''
	const readline = require('readline');

	const rl = readline.createInterface({
		input: process.stdin,
		output: process.stdout
	});

	rl.on('line', (line) => {
		if (line)
			data += line
	});

	rl.on('close', () => {
		contentType = contentType.split(';')
		test = decodeURIComponent(data?.replace('+', ' ')).split('&')
		if (contentType[0] === "multipart/form-data") {
			data = data.split('--' + contentType[1].split('=')[1])
			data.shift()
			data.pop()
			test = data.map(elm => {
				if (elm.indexOf('filename') < 0) {
					ret = elm.split('Content-Disposition: form-data; ').join('')
					ret = ret.substr(6).replace('"', ': ')
					return ret
				}
				else
					return ''
			})
		}

		displayRes(test)
	});
}
else {
	data = process.env.QUERY_STRING
	data = decodeURIComponent(data?.replace('+', ' ')).split('&')
	displayRes(data)
}