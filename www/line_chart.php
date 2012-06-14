<?php

// https://developers.google.com/chart/image/docs/gallery/line_charts?hl=fr-FR

require '../../lib/googlechartphplib-0.6/lib/GoogleChart.php';
require '../../lib/googlechartphplib-0.6/lib/markers/GoogleChartShapeMarker.php';
require '../../lib/googlechartphplib-0.6/lib/markers/GoogleChartTextMarker.php';


function display_chart($data)
{

  //echo "data : "; var_dump($data); echo "<br />";
  $i = 0;
  foreach($data as $abs=>$val)
  {
    //echo "$abs => $val <br />";
    $abscissa[$i] = $abs;
      //$arr[month_to_string($res[0])] = $res[1];
    $i++;
  }
  $maxval = max($data);
 
  $chart = new GoogleChart('lc', 600, 300);
  $chart->setScale(0,$maxval);
 
  $line = new GoogleChartData($data);
  $chart->addData($line);
 
  $y_axis = new GoogleChartAxis('y');
  $y_axis->setDrawTickMarks(false);
  $y_axis->setRange(0, $maxval);
//  $y_axis->setLabels(array(0,10,20,30)); 
  $chart->addAxis($y_axis);
 
  $x_axis = new GoogleChartAxis('x');
  $x_axis->setTickMarks(5);
  $x_axis->setLabels($abscissa);
  $chart->addAxis($x_axis);
 
  // add a shape marker with a border
  $shape_marker = new GoogleChartShapeMarker(GoogleChartShapeMarker::CIRCLE);
  $shape_marker->setSize(6);
  $shape_marker->setBorder(2);
  $shape_marker->setData($line);
  $chart->addMarker($shape_marker);

  // add a value marker
  $value_marker = new GoogleChartTextMarker(GoogleChartTextMarker::VALUE);
  $value_marker->setData($line);
  $chart->addMarker($value_marker);

  //    var_dump($chart->getQuery());
  //    echo $chart->validate();
  echo $chart->toHtml();
}

