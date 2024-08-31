#!/usr/bin/env php
<?php

$data = '';
$status = ["200", "OK"];

$methode = $_SERVER['REQUEST_METHOD'];
$dir = $_SERVER['UPLOAD_DIR'];

if (!strlen($dir) || !strlen($methode))
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
$disp = "";
foreach ($data as $key=>&$x) {
    $x = str_replace("=", ": ", urldecode($x));
    if ($methode == 'DELETE') {
        $tmp = explode(": ", $x);
        if ($tmp[0] == 'fileName') {
            $rem = $dir . $tmp[1];
            if (!file_exists($rem))
                exit(1);
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
            $status = ["204", "No Content"];
        }
    }
}

if ($status[0] == 204)
    $disp = join(" ", $status);
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
echo "HTTP/1.1 " . $status[0] . " " . $status[1] . "\r\nContent-Length: " . strlen($body) . "\r\nContent-Type: text/html\r\n\r\n" . $body;

?>