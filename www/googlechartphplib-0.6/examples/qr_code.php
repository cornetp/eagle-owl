<?php

require '../lib/GoogleQRCode.php';

$chart = new GoogleQRCode(150, 150);

$chart->setData('Hello world');
//~ $chart->setOutputEncoding('UTF-8');
//~ header('Content-Type: image/png');
echo $chart->toHtml();
