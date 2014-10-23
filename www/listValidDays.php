<?php
header("Content-Type: text/xml");

$xml = new SimpleXMLElement('<list/>');

$year = (isset($_POST["year"])) ? $_POST["year"] : "2010"/*NULL*/;
$month = (isset($_POST["month"])) ? $_POST["month"] : "1"/* NULL*/;

if ($year && $month)
{
  $config = parse_ini_file('/etc/eagleowl.conf', true);
  
  $root_path = "";
  $db_subdir = "";
  $stat_db   = "eagleowl_stat.db";
  
  if(isset($config['install_path']))
    $root_path = $config['install_path'];
  if(isset($config['db_subdir']))
    $db_subdir = $config['db_subdir'];
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
  
  $stat_db = $root_path."/".$db_subdir."/".$stat_db;
  
  $db = new SQLite3($stat_db);
  $req = "SELECT day, status FROM energy_hour_stat WHERE year = $year and month = $month GROUP BY year,month, day ;";
  $db->busyTimeout (10000);
  $result = $db->query($req);
  while ($res = $result->fetchArray())
  {
    $item = $xml->addChild('day', $res[0]);
    $item->addChild('status', $res[1]);
  }
print($xml->asXML());
}
else
{
  echo "FAIL";
}

?>
