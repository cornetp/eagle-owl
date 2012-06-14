<?php

require '../lib/GoogleChart.php';

$values = array();
for ($i = 0; $i <= 10; $i += 1) {
	$values[] = rand(10,100);
}

$chart = new GoogleChart('bvs', 500, 200);
$chart->addData(new GoogleChartData($values));

$y_axis = new GoogleChartAxis('y');
$chart->addAxis($y_axis);

header('Content-Type: image/png');
echo $chart;
