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

function get_years($db)
{
  $i = 0;
  $years = array();
  $result = $db->query("select distinct year from energy_history;");
  while ($res = $result->fetchArray()) {
    $years[$i] = $res['year'];
    $i++;
  }
  return $years;
}

function get_months($db, $year)
{
  $i = 0;
  $a = array();
  $result = $db->query("select distinct month from energy_history where year = \"$year\";");
  while ($res = $result->fetchArray()) {
    $a[$i] = $res['month'];
    $i++;
  }
  return $a;
}

function get_data($db, $year=0, $month=0, $day=0)
{
  $i = 0;
  $arr = array();

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
  while ($res = $result->fetchArray()) {
    $arr[$res[0]] = $res[1];
    $i++;
  }
  return $arr;
}

$db = new SQLite3('be.db');
/*
echo "different years: <br />";
$years = get_years($db);
foreach($years as $year)
{
  echo "$year <br />";
  $months = get_months($db, $year);
  foreach($months as $m)
    echo " => $m <br />";
}
*/

$year = 0;
$month= 0;
$day  = 0;
if(isset($_GET['year']))
  $year = $_GET['year'];
if(isset($_GET['month']))
  $month = $_GET['month'];
if(isset($_GET['day']))
  $day = $_GET['day'];

require 'line_chart.php';

$all_lnk = "<a href = .>All</a>";
$title = "$all_lnk";
if($year){
  $year_lnk = "<a href = ?year=$year>$year</a>";
  $title .= "/$year_lnk";
}
if($month){
  $month_lnk = "<a href = ?year=$year&month=$month>$month</a>";
  $title .= "/$month_lnk";
}
if($day){
  $day_lnk = "<a href = ?year=$year&month=$month&day=$day>$day</a>";
  $title .= "/$day_lnk";
}

echo "<h2>$title</h2>";


$data = get_data($db, $year, $month, $day);
display_chart($data);
echo "<br/>";
foreach($data as $abs=>$val)
{
  $abs_unit = 'year';
  $lnk = "<a href = ?";
  if($year){
    $lnk .= "year=$year&";
    $abs_unit = 'month';
  }
  if($month){
    $lnk .= "month=$month&";
    $abs_unit = 'day';
  }
  if($day){
    $lnk .= "day=$day&";
    $abs_unit = 'hour';
  }
  $lnk.= "$abs_unit=$abs";
  $lnk.=">$abs</a> ";
  echo "$lnk ";
}

echo "<br />";

/*
echo "<h2>2011-2012</h2>";
$data = get_data($db, 2011);
display_chart($data);
$data = get_data($db, 2012);
display_chart($data);
echo "<br />";

echo "<h2>01/2012</h2>";
$data = get_data($db, 2012, 01);
display_chart($data);
echo "<br />";

echo "<h2>12/05/2012</h2>";
$data = get_data($db, 2012, 05, 12);
display_chart($data);
echo "<br />";
*/
?>
