<?php
$addr =  intval($_GET['addr']);
$config = parse_ini_file('/etc/eagleowl.conf', true);
$root_path = "";
if(isset($config['install_path']))
  $root_path = $config['install_path'];
$live_file = $root_path."/.".$addr.".live";
echo file_get_contents($live_file);
?>
