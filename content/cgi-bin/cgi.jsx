#!/usr/bin/env node

meth = process.env.REQUEST_METHOD
if (!meth)
	exit(1)

const displayRes = (formD) => {
	formD = formD?.replace('+', ' ').split('&').map(a => {return decodeURIComponent(a)})
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

const fs = require('fs');
const { exit } = require('process')

if (meth == 'POST') {
	data = ''
	const readline = require('readline');
	const myFile = require('fs');

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
		exit(1)
	displayRes(data)
}