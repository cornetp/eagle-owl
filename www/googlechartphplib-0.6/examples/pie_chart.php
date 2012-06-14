<?php

require '../lib/GooglePieChart.php';

$values = array('Success' => 20, 'Failure' => 75, 'Unknow' => 5);

//~ $chart = new GooglePieChart('pc', 500, 200);

//~ $data = new GoogleChartData($values);
//~ $data->setLabelsAuto();
//~ $data->setLegend('Foo');
//~ $chart->addData($data);

//~ $data = new GoogleChartData(array(50,50));
//~ $data->setLabels(array('Foo','Bar'));
//~ $data->setLegend('Foo');
//~ $chart->addData($data);

$chart = new GooglePieChart('pc', 500, 200);
$chart->addData(new GoogleChartData(array(10,20,30)));

$data = new GoogleChartData(array(50,50));
$chart->addData($data);
		
$chart->setQueryMethod(GoogleChartApi::GET);
$data->setLabels(array('Foo','Bar'));

if ( isset($_GET['debug']) ) {
	var_dump($chart->getQuery());
	echo $chart->validate();
	echo $chart->toHtml();
}
else {
	header('Content-Type: image/png');
	echo $chart;
}

