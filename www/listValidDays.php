<?php
header("Content-Type: text/xml");

$xml = new SimpleXMLElement('<list/>');

$year = (isset($_POST["year"])) ? $_POST["year"] : NULL;
$month = (isset($_POST["month"])) ? $_POST["month"] : NULL;

if ($year && $month)
{
  $db = new SQLite3('/home/cornetp/vagrant/lucid32/eagle-owl/src/db/eagleowl_stat.db');
  $req = "SELECT day FROM energy_day_stat WHERE year = $year and month = $month;";
  $result = $db->query($req);
  while ($res = $result->fetchArray())
  {
    $item = $xml->addChild('day', $res[0]);
  }
print($xml->asXML());
}
else
{
  echo "FAIL";
}

?>
