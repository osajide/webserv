#!/usr/bin/env php
<?php

$data = '';
$methode = $_SERVER['REQUEST_METHOD'];

if ($methode != 'GET') {
    while (($line = fgets(STDIN)) !== false) {
        $data .= $line;
    }
    $data = urldecode($data);
}
else {
    $data = urldecode($_SERVER['QUERY_STRING']);
}

$data = explode('&', urldecode($data));
$data[2] = str_replace("=", ": ", $data[2]);
$data[3] = str_replace("=", ": ", $data[3]);
$data[4] = str_replace("=", ": ", $data[4]);

$body = <<<EOD
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
            <h1>Methode: {$methode} </h1>
            <h2>{$data[2]}</h2>
            <h2>{$data[3]}</h2>
            <h2>{$data[4]}</h2>
        </div>
    </body>
    </html>
EOD;

echo "HTTP/1.1 200 OK\r\nContent-Length: " . strlen($body) . "\r\nContent-Type: text/html\r\n\r\n" . $body;

?>