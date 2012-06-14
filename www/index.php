<html>
<head>

<title>JSChart</title>

<script type="text/javascript" src="JSCharts3_demo/sources/jscharts.js"></script>

</head>
<body>

<?php
function month_to_string($nb)
{
  $months = array(
    01 => 'Janvier',
    02 => 'Fevrier',
    03 => 'Mars',
    04 => 'Avril',
    05 => 'Mai',
    06 => 'Juin',
    07 => 'Juillet',
    08 => 'Aout',
    09 => 'Septembre',
    10 => 'Octobre',
    11 => 'Novembre',
    12 => 'Decembre',
  );
  return $months[$nb];
}


function html_to_js_var($t)
{
  return str_replace('</script>','<\/script>',addslashes(str_replace("\r",'',str_replace("\n","",$t))));
}
function var_to_js($jsname,$a)
{
  $ret='';
  if (is_array($a))
  {
    $ret.=$jsname.'= new Array();';

    foreach ($a as $k => $a)
    {
      if (is_int($k) || is_integer($k))
        $ret.= var_to_js($jsname.'['.$k.']',$a);
      else
        $ret.= var_to_js($jsname.'[\''.$k.'\']',$a);
    }
  }
  elseif (is_bool($a)) 
  {
    $v=$a ? "true" : "false";
    $ret.=$jsname.'='.$v.';';
  }
  elseif (is_int($a) || is_integer($a) || is_double($a) || is_float($a)) 
  {
    $ret.=$jsname.'='.$a.';';
  }
  elseif (is_string($a))
  {
    $ret.=$jsname.'=\''.html_to_js_var($a).'\';';
  }
  return $ret;
}

function get_data($db, $year=0, $month=0, $day=0)
{
  $i = 0;

  $unit = "year";
  if($year)
    $unit = "month";
  if($month)
    $unit = "day";
  if($day)
    $unit = "hour";
 
  $req = "SELECT ";
  $req.= "$unit, ";
  $req.= "SUM(ch1_kw_avg / 1000) FROM energy_history ";
  if($unit <> "year"){
    $req.= "WHERE ";
    if($year)  $req.= "year = \"$year\" ";
    if($month) $req.= "AND month = \"$month\" ";
    if($day)   $req.= "AND day = \"$day\" ";
  }
  $req.= "GROUP BY year";
  if($year)  $req.= ", month";
  if($month) $req.= ", day";
  if($day)   $req.= ", hour";
  $req.= ";";

//  echo "<br/>$req<br/><br/>";
  $result = $db->query($req);
  $arr = array();
  while ($res = $result->fetchArray())
  {
    $arr[$i] = array( 0 => month_to_string($res[0]), 1 => $res[1] );
    $i++;
  }
  return $arr;
}

$db = new SQLite3('../be.db');

$data = get_data($db, 2012);
$title = "Power consumption in 2012";

?>

<script language="JavaScript" type="text/javascript">

function callback(v) {
  alert('user click on "'+v+'"');
} 

function draw_chart(title)
{
	<?php echo var_to_js('myData', $data); ?>
//	var colors = ['#AF0202', '#EC7A00', '#FCD200', '#81C714', '#000', '#000'];

	var myChart = new JSChart('graph', 'bar');
	myChart.setDataArray(myData);
//	myChart.colorizeBars(colors);
	myChart.setTitle(title);
	myChart.setTitleColor('#FFFFFF');
	myChart.setAxisNameX('');
	myChart.setAxisNameY('kWh');
	myChart.setAxisColor('#AAAAFF');
	myChart.setAxisNameFontSize(16);
	myChart.setAxisNameColor('#FFFFFF');
	myChart.setAxisValuesColor('#FFFFFF');
	myChart.setBarValuesColor('#AAAAFF');
	myChart.setBarValuesFontSize(10);
	myChart.setBarValuesDecimals(2);
	myChart.setAxisPaddingTop(60);
	myChart.setAxisPaddingRight(20);
	myChart.setAxisPaddingLeft(60);
	myChart.setAxisPaddingBottom(40);
//	myChart.setTextPaddingLeft(105);
//	myChart.setTitleFontSize(11);
//	myChart.setBarBorderWidth(1);
	myChart.setBarBorderColor('#C4C4C4');
	myChart.setBarSpacingRatio(50);
//	myChart.setGrid(false);
	myChart.setSize(616, 321);
//	myChart.setBackgroundImage('chart_bg.jpg');
	myChart.setBackgroundColor('#222244');

/*
  var len=myData.length;
  for(var i=0; i<len; i++)
  {
    myChart.setTooltip([myData[i][0]], callback);
  }*/

	myChart.draw();
}
</script>

<?php
echo "<div id=\"graph\">Loading graph...</div>";
echo "<script language=javascript>draw_chart('$title')</script>";
?>
</body>
</html>
