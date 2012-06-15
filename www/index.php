<html>
<head>

<title>Electricity consumption</title>

<link rel="stylesheet" type="text/css" media="all" href="jsdatepick-calendar/jsDatePick_ltr.css" />
<script type="text/javascript" src="JSCharts3_demo/sources/jscharts.js"></script>
<script type="text/javascript" src="jsdatepick-calendar/jsDatePick.full.1.3.js"></script>

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
    if($unit == "month")
      $arr[$i] = array( 0 => month_to_string($res[0]), 1 => $res[1] );
    else
      $arr[$i] = array( 0 => "$res[0]", 1 => $res[1] );
    $i++;
  }
  return $arr;
}

$db = new SQLite3('be.db');

$year = 0;
$month= 0;
$day  = 0;
$graph_type = 'bar'; // graph type : bar, line or pie
$title = "All";
$axis_x_name = '';
if(isset($_GET['year']))
{
  $year = $_GET['year'];
  $title = "$year";
  $axis_x_name = 'month';
}
if(isset($_GET['month']))
{
  $month = $_GET['month'];
  $title = month_to_string($month)." $year";
  $axis_x_name = 'day';
}
if(isset($_GET['day']))
{
  $day = $_GET['day'];
  $graph_type = 'line';
  $title = "$day ".month_to_string($month)." $year";
  $axis_x_name = 'hour';
}

$data = get_data($db, $year, $month, $day);

// Following lines are to select a valid date in the calendar
if(!$year) $year = date('Y');
if(!$month) $month = 1;
if(!$day) $day = 1;
?>

<script language="JavaScript" type="text/javascript">

function callback(v) {
  alert('user click on "'+v+'"');
} 

function draw_chart(type, title, axis_x_name)
{
  <?php echo var_to_js('myData', $data); ?>
//var colors = ['#AF0202', '#EC7A00', '#FCD200', '#81C714', '#000', '#000'];

  //var myChart = new JSChart('graph', 'bar');
  var myChart = new JSChart('graph', type);
  myChart.setDataArray(myData);
//myChart.colorizeBars(colors);
  myChart.setTitle(title);
  myChart.setTitleColor('#FFFFFF');
  myChart.setAxisNameX(axis_x_name);
  myChart.setAxisNameY('kWh');
  myChart.setAxisColor('#AAAAFF');
  myChart.setAxisNameFontSize(16);
  myChart.setAxisNameColor('#FFFFFF');
  myChart.setAxisValuesColor('#FFFFFF');
  myChart.setBarValues(false);
//myChart.setBarValuesColor('#AAAAFF');
//myChart.setBarValuesFontSize(10);
//myChart.setBarValuesDecimals(2);
  myChart.setAxisPaddingTop(60);
  myChart.setAxisPaddingRight(20);
  myChart.setAxisPaddingLeft(60);
  myChart.setAxisPaddingBottom(40);
//myChart.setTextPaddingLeft(105);
//myChart.setTitleFontSize(11);
//myChart.setBarBorderWidth(1);
  myChart.setBarBorderColor('#C4C4C4');
  myChart.setBarSpacingRatio(50);
//myChart.setGrid(false);
  myChart.setSize(616, 321);
//myChart.setBackgroundImage('chart_bg.jpg');
  myChart.setBackgroundColor('#222244');
  myChart.setTooltipPosition('nw');

  var len=myData.length;
  for(var i=0; i<len; i++)
    myChart.setTooltip([myData[i][0], myData[i][1]]);

  myChart.draw();
}


  window.onload = function()
  {
    var year = <?php echo $year ?>;
    g_globalObject = new JsDatePick({
      useMode:1,
      isStripped:true,
      target:"div3_example",
      selectedDate:{ 
        day:<?php echo"$day" ?>,
        month:<?php echo"$month" ?>,
        year:<?php echo"$year" ?>},
      dateFormat:"%m-%d-%Y",
      imgPath:"jsdatepick-calendar/img/",
      weekStartDay:1
    });		
    
    g_globalObject.setOnSelectedDelegate(function(){
      var obj = g_globalObject.getSelectedDay();
      var url="index.php?year="+obj.year+"&month="+obj.month+"&day="+obj.day;
      window.open(url, "_self");
    });
    
    g_globalObject.setOnSelectedYearDelegate(function(){
      var obj = g_globalObject.getSelectedDay();
      var url="index.php?year="+obj.year;
      window.open(url, "_self");
    });
    
    g_globalObject.setOnSelectedMonthDelegate(function(){
      var obj = g_globalObject.getSelectedDay();
      var url="index.php?year="+obj.year+"&month="+obj.month;
      window.open(url, "_self");
    });
  
  };

</script>

<?php
echo "<div id=\"div3_example\" 
      style=\"margin:10px 0 30px 0; width:205px; height:230px;\"></div>";
echo "<div id=\"graph\">";
if(!$data)
  echo "No data for \"$title\"";
else
  echo "<script language=javascript>draw_chart('$graph_type', '$title','$axis_x_name')</script></div>";
?>
</body>
</html>
