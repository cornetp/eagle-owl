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
    1 => 'Janvier',
    2 => 'Fevrier',
    3 => 'Mars',
    4 => 'Avril',
    5 => 'Mai',
    6 => 'Juin',
    7 => 'Juillet',
    8 => 'Aout',
    9 => 'Septembre',
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
  $db->busyTimeout (10000);
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

function get_stat_data($db, $year=0, $month=0, $day=0)
{
  $i = 0;
  $unit = "year";
  $table = "energy_year_stat";
  if($year){
    $unit = "month";
    $table = "energy_month_stat";
  }
  if($month){
    $unit = "day";
    $table = "energy_day_stat";
  }
  if($day){
    $unit = "hour";
    $table = "energy_hour_stat";
  }

  $req = "SELECT ";
  $req.= "$unit, ";
  $req2 = "kwh_total, kwh_week_total, kwh_weekend_total FROM ".$table." ";
  if($unit <> "year"){
    $req2.= "WHERE ";
    if($year)  $req2.= "year = \"$year\" ";
    if($month) $req2.= "AND month = \"$month\" ";
    if($day){  
	  $req2.= "AND day = \"$day\" ";
	  $req2.="UNION SELECT hour+24, ".$req2."+1 AND hour=0 ";
	}
  }
    
  $req.= $req2."ORDER BY $unit;";

//  echo "<br/>$req<br/><br/>";
  $db->busyTimeout (10000);
  $result = $db->query($req);
  $arr = array();
  while ($res = $result->fetchArray())
  {
    if($unit == "month")
      $arr[$i] = array( 0 => month_to_string($res[0]), 
                        1 => $res[1],
                        2 => $res[2],
                        3 => $res[3] );
    else
      $arr[$i] = array( 0 => "$res[0]",
                        1 => $res[1],
                        2 => $res[2],
                        3 => $res[3] );
                        
    $i++;
  }
  return $arr;
}

function get_weekend_data($db, $year=0, $month=0, $day=0)
{
  $i = 0;
  if($year)
  {
      $table = "energy_year_stat";
    if($month)
      $table = "energy_month_stat";
    if($day)
      $table = "energy_day_stat";
    
    $req = "SELECT kwh_week_total, kwh_weekend_total FROM ".$table;
    $req.= " WHERE ";
    if($year)  $req.= "year = \"$year\" ";
    if($month) $req.= "AND month = \"$month\" ";
    if($day)   $req.= "AND day = \"$day\" ";
  }
  else
    $req = "SELECT sum(kwh_week_total), sum(kwh_weekend_total) FROM energy_year_stat";

  $req.= ";";
  
//  echo "<br/>$req<br/><br/>";
  $result = $db->query($req);
  $arr = array();
  $res = $result->fetchArray();
//  {
    $arr[0] = array( 0 => 'normal', 1 => $res[0]);
    $arr[1] = array( 0 => 'night & weekend', 1 => $res[1]);
//    $i++;
//  }
  return $arr;
}

$config = parse_ini_file('/etc/eagleowl.conf', true);

$root_path = "";
$db_subdir = "";
$main_db   = "eagleowl.db";
$stat_db   = "eagleowl_stat.db";

if(isset($config['install_path']))
  $root_path = $config['install_path'];
if(isset($config['db_subdir']))
  $db_subdir = $config['db_subdir'];
if(isset($config['main_db_name']))
  $main_db = $config['main_db_name'];
if(isset($config['stat_db_name']))
  $stat_db = $config['stat_db_name'];

if($root_path === "" || !is_dir($root_path))
{
  echo"invalid path \"$root_path\": set the correct install_path in /etc/eaglowl.conf";
  exit();
}
if(!is_dir($root_path."/".$db_subdir))
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

if($stat_db)
{
  $data = get_stat_data($stat_db, $year, $month, $day);
  $wedata = get_weekend_data($stat_db, $year, $month, $day);
}  
else
{
  $data = get_data($db, $year, $month, $day);
}

// Following lines are to select a valid date in the calendar
if(!$year) $year = date('Y');
if(!$month) $month = date('m');
//if(!$day) $day = date('d');
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
  if(type == 'bar')
  {
    myChart.setBarColor('#88FF88', 3)
    myChart.setBarColor('#FF8888', 2);
    myChart.setLegendForBar(1, 'Total');
    myChart.setLegendForBar(2, 'Jour');
    myChart.setLegendForBar(3, 'Nuit & week-end');
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
  var colors = ['#FF8888','#88FF88'];
  myChart.setDataArray(myData);
  myChart.colorizePie(colors);
  myChart.setTitle(title);
  myChart.setTitleColor('#FFFFFF');
  myChart.setSize(800, 200);
  myChart.setBackgroundColor('#222244');
  myChart.setLegend('#FF8888', 'Jour');
  myChart.setLegend('#88FF88', 'Nuit et week-end');
  myChart.setPieRadius(95);
  myChart.setShowXValues(false);
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
if(!$data)
  echo "No data for \"$title\"";
else{
  echo "<div id=\"graph\"><script language=javascript>draw_chart('$graph_type', '$title','$axis_x_name')</script></div>";
  if($wedata)
    echo "<div id=\"wegraph\"><script language=javascript>draw_we_chart('Tarifs','$axis_x_name')</script></div>";
}

?>
</body>
</html>
