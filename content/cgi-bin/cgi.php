#!/usr/bin/env php
<?php

$data = '';
$status = 200;
$methode = $_SERVER['REQUEST_METHOD'];
if (!strlen($methode))
    exit (1);
$dir = $_SERVER['UPLOAD_DIR'];
if (!strlen($dir))
    // exit (1);
    $dir = "/Users/ykhayri/Desktop/wsl_webserv/content/cgi-bin/uploads/";


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
$disp = "";
foreach ($data as $key=>&$x) {
    $x = str_replace("=", ": ", urldecode($x));
    if ($methode == 'DELETE') {
        $tmp = explode(": ", $x);
        if ($tmp[0] == 'fileName') {
            $rem = $dir . $tmp[1];
            if (!file_exists($rem)){
            // fwrite(STDERR, "====================================> This is an error message\n" . $rem);
                exit(1);
            }
        }
        else
            exit(1);
    }
    if ($key > 1)
        $disp .= "<h2>{$x}</h2>";
}
if ($methode == 'DELETE') {
    foreach ($data as $key=>&$x) {
        $x = str_replace("=", ": ", urldecode($x));
        $tmp = explode(": ", $x);
        if ($tmp[0] == 'fileName') {
            $rem = $dir . $tmp[1];
            if (!unlink($rem))
                exit(1);
            $status = 204;
        }
    }
}

$disp .= "<h2>{$s}</h2>";
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
            {$disp}
        </div>
    </body>
    </html>
EOD;
if ($status == 204)
    $body = '';
echo "HTTP/1.1 " . $status . " OK\r\nContent-Length: " . strlen($body) . "\r\nContent-Type: text/html\r\n\r\n" . $body;

?>