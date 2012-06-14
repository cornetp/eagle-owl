<?php

require '../lib/GoogleChart.php';
require '../lib/markers/GoogleChartTextMarker.php';
require '../lib/markers/GoogleChartShapeMarker.php';

$values = array(
	array(),
	array(),
	array()
);
$n = 10;
for ($i = 0; $i <= $n; $i += 1) {
	$v = rand($i , $i*10);
	$values[0][] = $v;
	$values[1][] = $v - $i;
	$values[2][] = rand(100 - ($i+10),100 - 10*$i);
}

$chart = new GoogleChart('lc', 600, 300);
$chart->setGridLines(10,10);
$chart->setLegendPosition('r');
//~ $chart->setMargin(50);
$chart->setLegendSize(150, 20);
$chart->setFill('ffffcc');
$chart->setGradientFill(45, array('cccccc', 'ffffff', 'cccccc'), GoogleChart::CHART_AREA);
$chart->setTitle('Us versus the others.');
$chart->setTitleColor('999999')->setTitleSize(20);

$line = new GoogleChartData($values[0]);
$line->setLegend('Us');
$chart->addData($line);

$marker = new GoogleChartShapeMarker(GoogleChartShapeMarker::X);
$marker->setData($line);
$marker->setColor('6699cc');
$chart->addMarker($marker);

$marker = new GoogleChartTextMarker(GoogleChartTextMarker::VALUE);
$marker->setData($line);
$chart->addMarker($marker);

$line = new GoogleChartData($values[1]);
$line->setDash(2,2);
$line->setColor('6699cc');
$chart->addData($line);


$line = new GoogleChartData($values[2]);
$line->setLegend('The others');
$line->setColor('ff0000');
$chart->addData($line);

$marker = new GoogleChartShapeMarker(GoogleChartShapeMarker::CIRCLE);
$marker->setData($line);
$marker->setColor('ff0000');
$chart->addMarker($marker);

$y_axis = new GoogleChartAxis('y');
$chart->addAxis($y_axis);

$x_axis = new GoogleChartAxis('x');
$x_axis->setTickMarks(5);
$x_axis->setDrawLine(false);
$x_axis->setTickColor('ff0000');
$chart->addAxis($x_axis);

if ( isset($_GET['debug']) ) {
	var_dump($chart->getQuery());
	echo $chart->validate();
	echo $chart->toHtml();
}
else{
	header('Content-Type: image/png');
	echo $chart;
}

