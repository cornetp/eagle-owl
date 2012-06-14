<?php

require '../lib/GooglePieChart.php';

$chart = new GooglePieChart('p', 130, 100);
$chart->setDataFormat(GoogleChart::SIMPLE_ENCODING);
$data = new GoogleChartData(array(80,-20));
$data->setColor('f9f900');
$chart->addData($data);

// I pass null to enable the "legend" trick
$data = new GoogleChartData(null);
$data->setColor('ffffff');
$data->setLegend('O O O');
$chart->addData($data);

$chart->setLegendPosition('r');
$chart->setRotation(0.628);

if ( isset($_GET['debug']) ) {
	var_dump($chart->getQuery());
	echo $chart->validate();
	echo $chart->toHtml();
}
else {
	header('Content-Type: image/png');
	echo $chart;
}

