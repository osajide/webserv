#!/usr/bin/env node

const fileChecker = require('fs');

meth = process.env.REQUEST_METHOD
dir = process.env.UPLOAD_DIR
if (!meth || !dir)
	process.exit(1)
statusCode = ["200", "OK"];

const displayRes = (formD) => {
	// bodyz = ''
	formD = formD?.replace('+', ' ').split('&').map(a => { return decodeURIComponent(a) })
	if (meth == 'DELETE') {
		formD.map(a => {
			tmp = a.split("=")
			if (tmp[0] === 'fileName') {
				rm = dir + tmp[1];
				if (!fileChecker.existsSync(rm))
					process.exit(1);
			}
			else
				process.exit(1);
		})
		formD.map(a => {
			filePath = dir + a.split("=")[1]
			ret = fileChecker.unlink(filePath, (err) => {
				if (err) {
					process.exit(1)
				}
			});
			if (!ret)
				statusCode = ["204", "No Content"]
		})
	}
	// else
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
					${meth != 'DELETE' ? (formD.map((a, index) => {
						if (index > 1 && a.length > a.indexOf('=') + 1)
							return `<h2>${a?.replace('=', ': ')}</h2>`}).join('')) : statusCode.join(' ')
					}
				</div>
			</body>
		
			</html>
		`
	console.log(`HTTP/1.1 ${statusCode.join(' ')}\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n\r\n${body}`)
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
			data += line + '\n'
	});

	rl.on('close', () => {
		displayRes(data)
	});
}
else {
	data = process.env.QUERY_STRING
	if (!data)
		process.exit(1)
	displayRes(data)
}