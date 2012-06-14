<?php

require '../../lib/GoogleBarChart.php';
?>

<h2>Data autoscaling (text format)</h2>
<?
$values1 = array(-10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110);
$values2 = array(10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130);

$chart = new GoogleBarChart('bvg', 500, 200);
$chart->setDataFormat(GoogleChart::TEXT);
$chart->setAutoscale(GoogleChart::AUTOSCALE_OFF);

$data = new GoogleChartData($values1);
$data->setAutoscale(true);
$chart->addData($data);

$data = new GoogleChartData($values2);
$data->setColor('336699');
$data->setAutoscale(true);
$chart->addData($data);

$y_axis = new GoogleChartAxis('y');
$chart->addAxis($y_axis);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Data autoscaling (simple encoding)</h2>
<?
$chart->setDataFormat(GoogleChart::SIMPLE_ENCODING);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Data autoscaling (extended encoding)</h2>
<?
$chart->setDataFormat(GoogleChart::EXTENDED_ENCODING);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>
