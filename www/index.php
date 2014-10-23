<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=utf-8" >.
<title>Electricity consumption</title>

<link rel="stylesheet" type="text/css" media="all" href="jsdatepick-calendar/jsDatePick_ltr.css" />
<script type="text/javascript" src="JSCharts3_demo/sources/jscharts.js"></script>
<script type="text/javascript" src="jsdatepick-calendar/jsDatePick.full.1.3.js"></script>

</head>
<body>

<?php
setlocale(LC_ALL, 'pl_PL.utf-8');
require "prices.php";
require "tariffs.php";
if (isset($_GET['tariff'])) {
  $tariff=$_GET['tariff'];
} else {
  $tariff="G12w";
}
function month_to_string($nb)
{
  return strftime("%B", mktime(0,0,0,$nb));
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

function get_data($db, $year=0, $month=0, $day=0, $addr=0) {
  $i = 0;
  $unit = "year";
  if ($year)
    $unit = "month";
  if ($month)
    $unit = "day";
  if ($day)
    $unit = "hour";
 
  $req = "SELECT ";
  $req.= "$unit, ";
  $req.= "SUM(ch1_kw_avg / 1000) FROM energy_history ";
  if ($unit <> "year"){
    $req.= "WHERE ";
    if ($year)  $req.= "year = \"$year\" ";
    if ($month) $req.= "AND month = \"$month\" ";
    if ($day)   $req.= "AND day = \"$day\" ";
  }
  if ($addr) {
    $req.="AND addr = \"$addr\" ";
  }
  $req.= "GROUP BY year";
  if ($year)  $req.= ", month";
  if ($month) $req.= ", day";
  if ($day)   $req.= ", hour";
  $req.= ";";

//  echo "<br/>$req<br/><br/>";
  $db->busyTimeout (10000);
  $result = $db->query($req);
  $arr = array();
  while ($res = $result->fetchArray())
  {
    if ($unit == "month")
      $arr[$i] = array( 0 => month_to_string($res[0]), 1 => $res[1] );
    else
      $arr[$i] = array( 0 => "$res[0]", 1 => $res[1] );
    $i++;
  }
  return $arr;
}

function get_stat_data($db, $year=0, $month=0, $day=0, $addr=0) {
  global $tariffs,$tariff,$prices;
  $i = 0;
  $unit = "year";
  $table = "energy_hour_stat";
  if ($year){
    $unit = "month";
  }
  if ($month){
    $unit = "day";
  }
  if ($day){
    $unit = "hour";
  }

  $req = "SELECT year, month, day, hour, dow, SUM(kwh_total) FROM energy_hour_stat ";
  if ($unit <> "year"){
    $req2.= "WHERE ";
    if ($year)  $req2.= "year = \"$year\" ";
    if ($month) $req2.= "AND month = \"$month\" ";
    if ($addr)  $req2.= "AND addr = \"$addr\" ";
    if ($day){  
  	  $req2.= "AND day = \"$day\" ";
	//    $req2.="UNION SELECT hour+24, ".$req2."+1 AND hour=0 ";
  	}
    if ($addr) 
      $reg2.= "AND addr = \"$addr\" ";
  }
  $req.= $req2."GROUP BY".($addr ? " addr," : "")." year, month, day, hour, dow ORDER BY year, month, day, hour;";
//  echo "<br/>$req<br/><br/>";
  $db->busyTimeout (10000);
  $result = $db->query($req);
  $arr = array();
  while ($res = $result->fetchArray()) {
    switch($unit) {
      case 'year':
        $index = $res[0];
        $arr[$index][0] = "$res[0]";
        break;
      case 'month':
        $index = $res[1];
        $arr[$index][0] =  month_to_string($res[1]);
        break;      
      case 'day':
        $index = $res[2];
        $arr[$index][0] = "$res[2]";
        break;
      case 'hour':
        $index = $res[3];
        $arr[$index][0] = "$res[3]";
        break;
      default:
    }
    $arr[$index][1] += $res[5];
    $arr[$index][2] +=0;
    $arr[$index][3] +=0;
    $arr[$index][4] +=0;
    $arr[$index][$tariffs[$tariff][$res[1]][$res[4]][$res[3]]+2] += $res[5];
  }
  foreach($arr as $key=>$item) {
    $val1+=$item[2];
    $val2+=$item[3];
    $val3+=$item[4];
  }
   $arr2[0] = array( 0 => round($prices[$tariff][0]*$val1/1000,2)." zł", 1 => $val1);
   $arr2[1] = array( 0 => round($prices[$tariff][1]*$val2/1000,2)." zł", 1 => $val2);
   $arr2[2] = array( 0 => round($prices[$tariff][2]*$val3/1000,2)." zł", 1 => $val3);

  $all['bar']=array_values($arr);
  $all['pie']=$arr2;
  $all['pietitle']=round($prices[$tariff][0]*$val1/1000,2)+ round($prices[$tariff][1]*$val2/1000,2)+round($prices[$tariff][2]*$val3/1000,2)." zł";
  return $all;
}

$config = parse_ini_file('/etc/eagleowl.conf', true);

$root_path = "";
$db_subdir = "";
$main_db   = "eagleowl.db";
$stat_db   = "eagleowl_stat.db";

if (isset($config['install_path']))
  $root_path = $config['install_path'];
if (isset($config['db_subdir']))
  $db_subdir = $config['db_subdir'];
if (isset($config['main_db_name']))
  $main_db = $config['main_db_name'];
if (isset($config['stat_db_name']))
  $stat_db = $config['stat_db_name'];

if ($root_path === "" || !is_dir($root_path))
{
  echo"invalid path \"$root_path\": set the correct install_path in /etc/eaglowl.conf";
  exit();
}
if (!is_dir($root_path."/".$db_subdir))
{
  echo "invalid path \"$root_path/$db_subdir\": ";
  echo "set the correct install_path and db_subdir in /etc/eaglowl.conf";
  exit();
}

$main_db = $root_path."/".$db_subdir."/".$main_db;
$stat_db = $root_path."/".$db_subdir."/".$stat_db;

$db = new SQLite3($main_db);
$stat_db = new SQLite3($stat_db);

$year = 0;
$month= 0;
$day  = 0;
$graph_type = 'bar'; // graph type : bar, line or pie
$title = "Total";
$axis_x_name = '';
if (isset($_GET['addr'])) {
  $addr = intval($_GET['addr']);

}
if (isset($_GET['year']))
{
  $year = intval($_GET['year']);
  $title = "$year";
  $axis_x_name = 'month';
}
if (isset($_GET['month'])) {
  $month = intval($_GET['month']);
  $title = month_to_string($month)." $year";
  $axis_x_name = 'day';
}
if (isset($_GET['day'])) {
  $day = intval($_GET['day']);
  $graph_type = 'line';
  $title = "$day ".month_to_string($month)." $year";
  $axis_x_name = 'hour';
}

if ($stat_db) {
  $all = get_stat_data($stat_db, $year, $month, $day, $addr);
//  echo "<pre>";
//  print_r($all);
  $data = $all['bar'];
  $wedata = $all['pie'];
  $pietitle = $all['pietitle'];
} else {
  $data = get_data($db, $year, $month, $day, $addr);
}

// Following lines are to select a valid date in the calendar
if (!$year) $year = date('Y');
if (!$month) $month = date('m');
//if (!$day) $day = date('d');
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
//  myChart.setAxisPaddingTop(60);
//  myChart.setAxisPaddingRight(20);
  myChart.setAxisPaddingLeft(50);
  myChart.setAxisPaddingBottom(40);
//myChart.setTextPaddingLeft(105);
//myChart.setTitleFontSize(11);
//myChart.setBarBorderWidth(1);
  myChart.setBarBorderColor('#C4C4C4');
  myChart.setBarSpacingRatio(40);
  if (type == 'bar') {
    myChart.setBarColor('#FF8888', 2);
    myChart.setBarColor('#88FF88', 3)
    myChart.setBarColor('#8888FF', 4)
    myChart.setLegendForBar(1, 'Total');
    myChart.setLegendForBar(2, 'Rate 1');
    myChart.setLegendForBar(3, 'Rate 2');
    myChart.setLegendForBar(4, 'Rate 3');
    myChart.setLegendShow(true);
    myChart.setLegendPosition('top middle');
  	myChart.setBarSpeed(100);
  }
//myChart.setGrid(false);
  myChart.setSize(800, 400);
//myChart.setBackgroundImage('chart_bg.jpg');
  myChart.setBackgroundColor('#222244');
  myChart.setTooltipPosition('nw');
  myChart.setLineSpeed(100);

  var len=myData.length;
  for(var i=0; i<len; i++)
//    myChart.setTooltip([myData[i][0], myData[i][1]]);
    myChart.setTooltip([myData[i][0]]);

  myChart.draw();
}

function draw_we_chart(title, axis_x_name)
{
  <?php echo var_to_js('myData', $wedata); ?>

  var myChart = new JSChart('wegraph', 'pie');
  var colors = ['#FF8888','#88FF88','#8888FF'];
  myChart.setDataArray(myData);
  myChart.colorizePie(colors);
  myChart.setTitle(title);
  myChart.setTitleColor('#FFFFFF');
  myChart.setSize(800, 200);
  myChart.setBackgroundColor('#222244');
  myChart.setLegend('#FF8888', 'Rate 1');
  myChart.setLegend('#88FF88', 'Rate 2');
  myChart.setLegend('#8888FF', 'Rate 3');
  myChart.setPieRadius(95);
  myChart.setShowXValues(true);
  myChart.setLegendShow(true);
  myChart.setLegendFontFamily('Times New Roman');
  myChart.setLegendFontSize(10);
  myChart.setLegendPosition(550, 80);
  myChart.setPieValuesColor('#000000');
  myChart.setPieAngle(50);
  myChart.set3D(true);

  myChart.draw();
}


  window.onload = function()
  {
    var year = <?php echo $year ?>;
    g_globalObject = new JsDatePick({
      useMode:1,
      isStripped:true,
      target:"div_calendar",
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
//echo "<div id=\"div_calendar\" style=\"margin:10px 0 10px 0; width:205px; height:200px;\"></div>";
echo "<table><tr><td><div id=\"div_calendar\" style=\"margin:10px 0 10px 0; width:205px; height:210px;\"></td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td>";
echo  "<td><form> <input type=\"button\" value=\"Live consumption\" onclick=\"window.open('live.php')\"> </form></td></tr></div></table>";
echo "<div id=\"graph\">";
if (!$data)
  echo "No data for \"$title\"";
else{
  echo "<div id=\"graph\"><script language=javascript>draw_chart('$graph_type', '$title','$axis_x_name')</script></div>";
  if ($wedata)
    echo "<div id=\"wegraph\"><script language=javascript>draw_we_chart('$pietitle','$axis_x_name')</script></div>";
}

?>
</body>
</html>
