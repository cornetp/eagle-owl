<?php

require '../../lib/GoogleBarChart.php';
?>

<h2>No autoscaling</h2>
<?
$values1 = array(-10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110);
$values2 = array(10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130);

$chart = new GoogleBarChart('bvg', 500, 200);
$chart->setDataFormat(GoogleChart::TEXT);
$chart->setAutoscale(GoogleChart::AUTOSCALE_OFF);

$data1 = new GoogleChartData($values1);
$data1->setAutoscale(false);
$chart->addData($data1);

$data2 = new GoogleChartData($values2);
$data2->setColor('336699');
$data2->setAutoscale(false);
$chart->addData($data2);

$y_axis = new GoogleChartAxis('y');
$chart->addAxis($y_axis);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Chart-wide manual scaling (text encoding)</h2>
<?
$chart->setScale(-10,130);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Chart-wide manual scaling (simple encoding)</h2>
<?
$chart->setDataFormat(GoogleChart::SIMPLE_ENCODING);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Chart-wide manual scaling (extended encoding)</h2>
<?
$chart->setDataFormat(GoogleChart::EXTENDED_ENCODING);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>
