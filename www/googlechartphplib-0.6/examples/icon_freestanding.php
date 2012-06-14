<?php

require '../lib/icons/GoogleChartIconNote.php';

$chart = new GoogleChartIconNote('Hello world');
$chart->setTitle('Example');
$chart->setTextColor('D01F3C');

if ( isset($_GET['debug']) ) {
	var_dump($chart->getQuery());
	echo $chart->validate();
	echo $chart->toHtml();
}
else{
	header('Content-Type: image/png');
	echo $chart;
}

