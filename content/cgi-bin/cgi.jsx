#!/usr/bin/env node

meth = process.env.REQUEST_METHOD
contentType = process.env.CONTENT_TYPE
ex_code = 200

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

	console.log(`HTTP/1.1 ${ex_code} OK\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n\r\n${body}`)
}

const fs = require('fs');

// Function to create and fill a file with data
const createAndFillFile = (filePath, vals) => {
  try {
    // Check if the file already exists
    if (fs.existsSync(filePath)) {
      console.log('File already exists.');
      return;
    }

    // Create the file and write vals to it
    fs.writeFileSync(filePath, vals, 'utf8');
    console.log('File created and filled successfully.');
  } catch (err) {
    console.error('Error creating file:', err);
  }
}


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
			data += line
	});

	rl.on('close', () => {
		contentType = contentType.split(';')
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
				else {
					createAndFillFile('./uploads/myFile.txt', data);
					return ''
				}
			})
		}
		else
			test = data?.replace('+', ' ').split('&').map(a => {return decodeURIComponent(a)})

		displayRes(test)
	});
}
else {
	data = process.env.QUERY_STRING
	data = data?.replace('+', ' ').split('&').map(a => {return decodeURIComponent(a)})
	displayRes(data)
}