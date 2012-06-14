<?php

require '../lib/GoogleChart.php';

$sin = array();
$cos = array();
for ($i = 0; $i <= 360; $i += 10) {
	$sin[] = round(sin($i * M_PI / 180),2);
	$cos[] = round(cos($i * M_PI / 180),2);
}

$chart = new GoogleChart('lc', 500, 200);
$chart->setGridLines(25, 50, 1, 1);
$chart->setMargin(30, 50);
$chart->setLegendSize(100,10);
$chart->setFill('333333');
$chart->setFill('444444', GoogleChart::CHART_AREA);
$chart->setTitle('Sinus & Cosinus');
$chart->setTitleColor('FFFFFF');
$chart->setTitleSize(18);

$sin = new GoogleChartData($sin);
$sin->setLegend('Sinus');
$sin->setThickness(2);
$sin->setColor('D1F2A5');
$chart->addData($sin);

$cos = new GoogleChartData($cos);
$cos->setLegend('Cosinus');
$cos->setThickness(2);
$cos->setColor('F56991');
$chart->addData($cos);

$y_axis = new GoogleChartAxis('y');
$y_axis->setDrawLine(false);
$y_axis->setRange(-1,1);
$y_axis->setLabelColor('ffffff');
$chart->addAxis($y_axis);

$x_axis = new GoogleChartAxis('x');
$x_axis->setDrawLine(false);
$x_axis->setRange(0,360);
$x_axis->setLabels(array(0, 90, 180, 270, 360));
$x_axis->setLabelColor('ffffff');
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
