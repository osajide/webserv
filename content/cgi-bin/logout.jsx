#!/usr/bin/env node

const fs = require('fs')
const dbData = require('fs')
const dbPath = '../../DB/data.json'
const db = fs.readFileSync(dbPath, 'utf8');
const jsonData = JSON.parse(db);
const cookie = process.env.HTTP_COOKIE_session_id


cookies_headers = ''
data = ''
user = {}


const logout = (a = null) => {
	if (a !== null) {
		cookies_headers = `Set-Cookie: session_id=${jsonData["sessions"][a].sessId}; Max-Age=${0}}`
		delete jsonData["sessions"][a];
		dbData.writeFile(dbPath, JSON.stringify(jsonData, null, 4), (err) => {
		})
	}

	body = `
				<html>
				<head>
				<meta http-equiv="refresh" content="0; url=/cgi-bin/forms/login.html">
				</head>
				<body>
				</body>
				</html>
				`
	console.log(`HTTP/1.1 200 OK\r\nContent-Length: ${body.length}\r\nContent-Type: text/html\r\n${cookies_headers}\r\n\r\n${body}`)
}
Object.keys(jsonData["sessions"]).map(a => {
	if (jsonData["sessions"][a].sessId == cookie) {
			logout(a)
			return;
	}
})
