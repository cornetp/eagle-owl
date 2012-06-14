<?php

require '../lib/GoogleChart.php';
require '../lib/markers/GoogleChartTextMarker.php';

$values = array();
for ($i = 0; $i <= 10; $i += 1) {
	$values[] = rand(20,80);
}

$chart = new GoogleChart('bvs', 500, 200);
$chart->setScale(0,100);
$data = new GoogleChartData($values);
$chart->addData($data);

$marker = new GoogleChartTextMarker();
$marker->setData($data);
$chart->addMarker($marker);

header('Content-Type: image/png');
echo $chart;

