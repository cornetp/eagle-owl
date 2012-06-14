<?php

require '../lib/GoogleChart.php';
require '../lib/markers/GoogleChartLineMarker.php';

$values = array();
for ($i = 0; $i <= 10; $i += 1) {
	$values[] = rand(0,100);
}

$chart = new GoogleChart('bvs', 500, 200);
$data = new GoogleChartData($values);
$chart->addData($data);

$marker = new GoogleChartLineMarker();
$marker->setData($data);
$chart->addMarker($marker);

header('Content-Type: image/png');
echo $chart;
