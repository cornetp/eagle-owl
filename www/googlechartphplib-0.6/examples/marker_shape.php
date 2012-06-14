<?php

require '../lib/GoogleChart.php';
require '../lib/markers/GoogleChartShapeMarker.php';

$values = array();
for ($i = 0; $i <= 10; $i += 1) {
	$values[] = rand(20,80);
}

$chart = new GoogleChart('lc', 500, 200);
$chart->setScale(0,100);
$data = new GoogleChartData($values);
$chart->addData($data);

$marker = new GoogleChartShapeMarker(GoogleChartShapeMarker::CIRCLE);
$marker->setData($data);
$chart->addMarker($marker);

$marker = new GoogleChartShapeMarker(GoogleChartShapeMarker::ARROW);
$marker->setData($data);
$marker->setColor('ff0000');
$marker->setPoint(2);
$chart->addMarker($marker);

header('Content-Type: image/png');
echo $chart;
