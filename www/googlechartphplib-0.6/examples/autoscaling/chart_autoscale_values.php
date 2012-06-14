<?php

require '../../lib/GoogleBarChart.php';
?>

<h2>Chart autoscaling by values (text encoding)</h2>
<?
$values1 = array(-10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110);
$values2 = array(10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130);

$chart = new GoogleBarChart('bvg', 500, 200);
$chart->setDataFormat(GoogleChart::TEXT);
$chart->setAutoscale(GoogleChart::AUTOSCALE_VALUES);

$data = new GoogleChartData($values1);
$chart->addData($data);

$data = new GoogleChartData($values2);
$data->setColor('336699');
$chart->addData($data);

$y_axis = new GoogleChartAxis('y');
$chart->addAxis($y_axis);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>

<h2>Chart autoscaling by values (simple encoding)</h2>
<?
$chart->setDataFormat(GoogleChart::SIMPLE_ENCODING);

echo $chart->toHtml();
var_dump($chart->getQuery());
?>
