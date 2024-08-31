#!/usr/bin/env node

const fs = require('fs')
const dbData = require('fs')
const dbPath = '../../DB/data.json'
const db = fs.readFileSync(dbPath, 'utf8');
const jsonData = JSON.parse(db);

const meth = process.env.REQUEST_METHOD
const cookie = process.env.HTTP_COOKIE_session_id

cookies_headers = ''
data = ''
user = {}

const readline = require('readline');
const crypto = require('crypto');

function generateRandomString(length) {
	const randomBytes = crypto.randomBytes(length);

	return randomBytes.toString('hex');
}

const myBodyMyChoice = (email) => {

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
				<h1>Hello ${email}</h1>
				<img src="/cgi-bin/forms/assets/welcome.jpg"/>
			</div>
		</body>

		</html>
	`
	console.log(`HTTP/1.1 200 OK\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n${cookies_headers}\r\n\r\n${body}`)
}


if (meth != 'GET') {
	const rl = readline.createInterface({
		input: process.stdin,
		output: process.stdout
	});

	rl.on('line', (line) => {
		if (line)
			data += line;
	});

	rl.on('close', () => {
		data = data?.replace('+', ' ').split('&').map(a => { return decodeURIComponent(a) })
		data.map((d) => {
			const [a, b] = d.split('=')
			user[a] = b
		})

		Object.keys(jsonData["users"]).map(a => {
			if (jsonData["users"][a].email === user["Email"] && jsonData["users"][a].password === user["Password"]) {
				curr = new Date()
				durr = 3600
				jsonData["sessions"][a] = { sessId: generateRandomString(32), expire: curr.valueOf() + durr * 1000 }
				console.error(jsonData["sessions"][a])
				cookies_headers = `Set-Cookie: session_id=${jsonData["sessions"][a].sessId}; Max-Age=${durr}}`
			}
		})
		dbData.writeFile(dbPath, JSON.stringify(jsonData, null, 4), (err) => {
		})
		myBodyMyChoice(user["Email"])
	});
}
else {
	Object.keys(jsonData["sessions"]).map(a => {
		if (jsonData["sessions"][a].sessId == cookie) {
			curr = new Date()
			if (curr.valueOf() > jsonData["sessions"][a].expire  * 1000) {
				cookies_headers = `Set-Cookie: session_id=${jsonData["sessions"][a].sessId}; Max-Age=${0}}`
			}
			else
				myBodyMyChoice(jsonData["users"][a].email)
		}
	})
}
