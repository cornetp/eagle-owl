<?php

/**
 * This chart could use the undocumented parameter "lfi".
 * See http://cse-mjmcl.cse.bris.ac.uk/blog/2007/12/23/1198436217875.html
 */

require '../lib/GoogleChart.php';

$values = array(34,18,21,70,53,39,39,30,13,15,4,8,5,8,4,8,44,16,16,3,10,7,5,20,20,28,44);

$chart = new GoogleChart('ls', 75, 30);

$data = new GoogleChartData($values);
$data->setThickness(1);
$data->setColor('0077CC');
$data->setFill('E6F2FA');

$chart->addData($data);

if ( isset($_GET['debug']) ) {
	var_dump($chart->getQuery());
	echo $chart->validate();
	echo $chart->toHtml();
}
else{
	header('Content-Type: image/png');
	echo $chart;
}
