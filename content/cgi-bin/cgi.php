#!/usr/bin/env php
<?php

$data = '';
$methode = $_SERVER['REQUEST_METHOD'];
$contentType = $_SERVER['CONTENT_TYPE'];
$i = 4;

function dataRet($elm) {
    $ret = explode('Content-Disposition: form-data; ', $elm);
    $ret = substr(join('', $ret), 8);
    $ret = preg_replace('/\"/', ': ', $ret, 1);
    return $ret;
}

if ($methode === 'POST') {
    while (($line = fgets(STDIN)) !== false) {
        $data .= $line;
        $data .= "\n";
    }
    $i = 5;
    $contentType = explode(';', $contentType);
    if ($contentType[0] == "multipart/form-data") {
        $sep = "--";
        $sep .= explode('=', $contentType[1])[1];
        $data = explode($sep, $data);
        array_pop($data);
        array_shift($data);
        $data = array_map("dataRet", $data);
    }
    else{
        $data = urldecode($data);
        $data = explode('&', $data);
    }
}
else {
    $data = urldecode($_SERVER['QUERY_STRING']);
    $data = explode('&', $data);
    $i = 4;
}

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
            <h2>{$data[$i]}</h2>
        </div>
    </body>
    </html>
EOD;

echo "HTTP/1.1 200 OK\r\nContent-Length: " . strlen($body) . "\r\nContent-Type: text/html\r\n\r\n" . $body;

?>