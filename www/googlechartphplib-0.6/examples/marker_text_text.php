<?php

require '../lib/GoogleChart.php';
require '../lib/markers/GoogleChartTextMarker.php';
require '../lib/markers/GoogleChartShapeMarker.php';

$values = array();
for ($i = 0; $i <= 10; $i += 1) {
	$values[] = rand(20,80);
}

$chart = new GoogleChart('bvs', 500, 200);
$chart->setScale(0,100);
$data = new GoogleChartData($values);
$chart->addData($data);

$marker = new GoogleChartTextMarker(GoogleChartTextMarker::FLAG, 'Hello, world!');
$marker->setData($data);
$marker->setStep(2);
$chart->addMarker($marker);

// a fixed position marker
$marker = new GoogleChartTextMarker(GoogleChartTextMarker::TEXT, 'Here');
$marker->setFixedPosition(0.25,1);
$chart->addMarker($marker);

header('Content-Type: image/png');
echo $chart;

