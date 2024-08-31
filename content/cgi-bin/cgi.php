#!/usr/bin/env php
<?php

$data = '';
$methode = $_SERVER['REQUEST_METHOD'];
if (!strlen($methode))
    exit (1);

if ($methode === 'POST') {
    while (($line = fgets(STDIN)) !== false) {
        $data .= $line;
        $data .= "\n";
    }
}
else {
    $data = $_SERVER['QUERY_STRING'];
    if (!strlen($data))
        exit (1);
}
$data = explode('&', $data);
foreach ($data as &$x)
    $x = str_replace("=", ": ", urldecode($x));

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